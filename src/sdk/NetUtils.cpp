#include "NetUtils.h"
#include <WiFi.h>
#include <cstring>
#include <memory>
#include <lwip/sockets.h>
#include <lwip/inet.h>
#include <lwip/ip.h>
#include <netdb.h>
#include "../sdk/UiTemplates.h"

namespace {
static bool waitForStatusConnected(uint32_t timeoutMs) {
    uint32_t start = millis();
    while (millis() - start < timeoutMs) {
        if (WiFi.status() == WL_CONNECTED) {
            return true;
        }
        delay(50);
    }
    return WiFi.status() == WL_CONNECTED;
}
}

namespace NetUtils {

static bool gExitToMenuRequested = false;

bool consumeExitToMenuRequest() {
    const bool was = gExitToMenuRequested;
    gExitToMenuRequested = false;
    return was;
}

namespace {
// Simple ICMP echo packet
struct IcmpEcho {
    uint8_t type;      // 8 = echo request
    uint8_t code;      // 0
    uint16_t checksum;
    uint16_t identifier;
    uint16_t sequence;
};

static uint16_t icmpChecksum(const void *data, size_t len) {
    const uint16_t *p = static_cast<const uint16_t *>(data);
    uint32_t sum = 0;
    size_t n = len;

    while (n > 1) {
        sum += *p++;
        n -= 2;
    }
    if (n == 1) {
        sum += *reinterpret_cast<const uint8_t *>(p);
    }

    // Fold to 16 bits
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    return static_cast<uint16_t>(~sum);
}
} // namespace

bool ensureWiFi(Watchy &watchy, uint8_t retries, uint32_t attemptTimeoutMs) {
    for (uint8_t i = 0; i <= retries; ++i) {
        if (WiFi.status() == WL_CONNECTED) {
            return true;
        }
        // Use Watchy's built-in WiFi connector (honors settings/credentials).
        if (!watchy.connectWiFi()) {
            continue;
        }
        if (waitForStatusConnected(attemptTimeoutMs)) {
            return true;
        }
    }
    return WiFi.status() == WL_CONNECTED;
}

NetResponse httpGet(String &url, uint32_t timeoutMs, bool shutdownRadio) {
    NetResponse resp{};

    if (url.length() == 0) {
        resp.httpCode = -1;
        resp.error = "Empty URL";
        return resp;
    }

    // Manually follow redirects so behavior is consistent across core versions.
    String currentUrl = url;
    static constexpr uint8_t kMaxRedirects = 6;
    static constexpr uint8_t kMaxAttemptsPerHop = 2;
    static constexpr size_t kMaxBodyBytes = 340000;
    // When responses exceed our cap, keep BOTH a small head and a rolling tail.
    // Some sites front-load large CSS/JS, and the useful content may be in the middle,
    // not necessarily the last bytes.
    static constexpr size_t kHeadBytes = 64 * 1024;
    static constexpr size_t kTailBytes = (kMaxBodyBytes > kHeadBytes) ? (kMaxBodyBytes - kHeadBytes) : 0;

    auto resolveRedirect = [](const String &fromUrl, const String &location) -> String {
        if (location.length() == 0) {
            return String();
        }
        // Absolute URL.
        if (location.startsWith("http://") || location.startsWith("https://")) {
            return location;
        }

        // Build authority: scheme://host[:port]
        int schemePos = fromUrl.indexOf("://");
        if (schemePos < 0) {
            return String();
        }
        const int authorityStart = schemePos + 3;
        int pathStart = fromUrl.indexOf('/', authorityStart);
        String authority = (pathStart < 0) ? fromUrl : fromUrl.substring(0, pathStart);

        // Root-relative.
        if (location[0] == '/') {
            return authority + location;
        }

        // Path-relative.
        if (pathStart < 0) {
            return authority + "/" + location;
        }
        int lastSlash = fromUrl.lastIndexOf('/');
        if (lastSlash < authority.length()) {
            return authority + "/" + location;
        }
        return fromUrl.substring(0, lastSlash + 1) + location;
    };

    auto doSingleGet = [&](const String &requestUrl) -> NetResponse {
        NetResponse r{};

        HTTPClient http;
        http.setConnectTimeout(timeoutMs);
        http.setTimeout(timeoutMs);

        // Some public endpoints respond differently without a browser-like user-agent.
        http.setUserAgent("Mozilla/5.0 (Watchy; ESP32) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36");

        // Capture response headers that help us follow redirects and diagnose failures.
        const char *headerKeys[] = {"Location", "Content-Type", "Content-Encoding"};
        http.collectHeaders(headerKeys, 3);

        const bool isHttps = requestUrl.startsWith("https://");
        bool began = false;
        // IMPORTANT: the client instance must outlive the whole request.
        // HTTPClient stores a pointer to the client passed to begin().
        WiFiClientSecure secureClient;
        if (isHttps) {
            secureClient.setInsecure();
            began = http.begin(secureClient, requestUrl);
        } else {
            began = http.begin(requestUrl);
        }

        if (!began) {
            r.httpCode = -1;
            r.error = "HTTP begin failed";
            return r;
        }

        // Many microcontroller HTTP stacks don't support gzip/br decoding.
        // Force an uncompressed payload.
        http.addHeader("Accept-Encoding", "identity", false, true);
        http.addHeader("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
        http.addHeader("Connection", "close");

        const int code = http.GET();
        r.httpCode = code;
        r.location = http.header("Location");
        r.contentType = http.header("Content-Type");
        r.contentEncoding = http.header("Content-Encoding");

        if (code <= 0) {
            r.error = String("GET failed: ") + http.errorToString(code).c_str();
            http.end();
            return r;
        }

        if (code != HTTP_CODE_OK) {
            r.error = String("HTTP error: ") + code;
            http.end();
            return r;
        }

        // Read incrementally with a safety cap to avoid allocation failures.
        // For large pages where the useful content is late in the document (e.g. CNN Lite),
        // we keep a rolling tail buffer rather than truncating early.
        r.body = "";
        const int announcedSize = http.getSize();
        if (announcedSize > 0) {
            const size_t reserveBytes = (static_cast<size_t>(announcedSize) < kMaxBodyBytes)
                                            ? static_cast<size_t>(announcedSize)
                                            : kMaxBodyBytes;
            r.body.reserve(reserveBytes + 1);
        } else {
            r.body.reserve(2048);
        }

        WiFiClient *stream = http.getStreamPtr();
        int remaining = announcedSize; // may be -1 when unknown/chunked
        uint8_t buf[256];
        uint32_t lastDataMs = millis();

        bool usingTail = false;
        String head;
        std::unique_ptr<uint8_t[]> tailBuf;
        size_t tailPos = 0;
        size_t tailLen = 0;

        auto tailWrite = [&](const uint8_t *p, size_t n) {
            if (kTailBytes == 0) {
                return;
            }
            if (!tailBuf) {
                tailBuf.reset(new uint8_t[kTailBytes]);
            }
            for (size_t i = 0; i < n; ++i) {
                tailBuf[tailPos] = p[i];
                tailPos = (tailPos + 1) % kTailBytes;
                if (tailLen < kTailBytes) {
                    ++tailLen;
                }
            }
        };

        // IMPORTANT: http.connected() can become false before we drain buffered data,
        // especially over TLS. Continue while data is still available.
        while ((http.connected() || stream->available() > 0) && (remaining > 0 || remaining == -1)) {
            const size_t avail = stream->available();
            if (avail == 0) {
                if (millis() - lastDataMs > timeoutMs) {
                    break;
                }
                delay(0);
                continue;
            }

            size_t toRead = avail;
            if (toRead > sizeof(buf)) {
                toRead = sizeof(buf);
            }
            if (remaining > 0 && toRead > static_cast<size_t>(remaining)) {
                toRead = static_cast<size_t>(remaining);
            }

            const size_t readCount = stream->readBytes(buf, toRead);
            if (readCount == 0) {
                delay(0);
                continue;
            }

            lastDataMs = millis();

            if (!usingTail) {
                if (r.body.length() + readCount <= kMaxBodyBytes) {
                    r.body.concat(reinterpret_cast<const char *>(buf), static_cast<unsigned int>(readCount));
                } else {
                    // Switch to head+tail mode:
                    // - preserve the first kHeadBytes bytes
                    // - keep a rolling window of the last kTailBytes bytes
                    usingTail = true;
                    head = (r.body.length() > kHeadBytes) ? r.body.substring(0, kHeadBytes) : r.body;
                    if (r.body.length() > head.length()) {
                        tailWrite(reinterpret_cast<const uint8_t *>(r.body.c_str() + head.length()),
                                  static_cast<size_t>(r.body.length() - head.length()));
                    }
                    r.body = ""; // no longer used until final assembly
                    tailWrite(buf, readCount);
                }
            } else {
                tailWrite(buf, readCount);
            }

            if (remaining > 0) {
                remaining -= static_cast<int>(readCount);
            }
        }

        if (usingTail) {
            String tailOut;
            if (tailBuf && tailLen > 0) {
                tailOut.reserve(tailLen + 1);
                const size_t start = (tailLen == kTailBytes) ? tailPos : 0;
                const size_t firstSeg = (start + tailLen <= kTailBytes) ? tailLen : (kTailBytes - start);
                tailOut.concat(reinterpret_cast<const char *>(tailBuf.get() + start), static_cast<unsigned int>(firstSeg));
                const size_t secondSeg = tailLen - firstSeg;
                if (secondSeg > 0) {
                    tailOut.concat(reinterpret_cast<const char *>(tailBuf.get()), static_cast<unsigned int>(secondSeg));
                }
            }

            // Assemble final body: head + tail window.
            // Insert a newline separator to reduce the chance of glueing partial tags together.
            r.body = head;
            if (tailOut.length() > 0) {
                r.body += "\n";
                r.body += tailOut;
            }
        }

        http.end();
        return r;
    };

    for (uint8_t redirects = 0; redirects <= kMaxRedirects; ++redirects) {
        NetResponse hopResp{};

        for (uint8_t attempt = 0; attempt < kMaxAttemptsPerHop; ++attempt) {
            hopResp = doSingleGet(currentUrl);

            // Retry only on transport-level failures.
            if (hopResp.httpCode <= 0 && attempt + 1 < kMaxAttemptsPerHop) {
                delay(120);
                continue;
            }
            break;
        }

        resp = hopResp;

        if (resp.httpCode == HTTP_CODE_MOVED_PERMANENTLY || resp.httpCode == HTTP_CODE_FOUND ||
            resp.httpCode == HTTP_CODE_SEE_OTHER || resp.httpCode == HTTP_CODE_TEMPORARY_REDIRECT ||
            resp.httpCode == HTTP_CODE_PERMANENT_REDIRECT) {
            const String nextUrl = resolveRedirect(currentUrl, resp.location);
            if (nextUrl.length() == 0) {
                resp.error = String("Redirect without valid Location (HTTP ") + resp.httpCode + ")";
                break;
            }
            if (redirects == kMaxRedirects) {
                resp.error = "Too many redirects";
                break;
            }
            currentUrl = nextUrl;
            continue;
        }

        break;
    }

    // Expose the final resolved URL (after any redirects) to callers.
    url = currentUrl;

    if (shutdownRadio) {
        WiFi.mode(WIFI_OFF);
        btStop();
    }

    return resp;
}

NetResponse httpPostForm(String &url,
                         const String &formBody,
                         const String &origin,
                         const String &referer,
                         const String &acceptLanguage,
                         uint32_t timeoutMs,
                         bool shutdownRadio) {
    NetResponse resp{};

    if (url.length() == 0) {
        resp.httpCode = -1;
        resp.error = "Empty URL";
        return resp;
    }

    String currentUrl = url;
    static constexpr uint8_t kMaxRedirects = 6;
    static constexpr uint8_t kMaxAttemptsPerHop = 2;
    static constexpr size_t kMaxBodyBytes = 340000;
    static constexpr size_t kHeadBytes = 64 * 1024;
    static constexpr size_t kTailBytes = (kMaxBodyBytes > kHeadBytes) ? (kMaxBodyBytes - kHeadBytes) : 0;

    auto resolveRedirect = [](const String &fromUrl, const String &location) -> String {
        if (location.length() == 0) {
            return String();
        }
        if (location.startsWith("http://") || location.startsWith("https://")) {
            return location;
        }
        int schemePos = fromUrl.indexOf("://");
        if (schemePos < 0) {
            return String();
        }
        const int authorityStart = schemePos + 3;
        int pathStart = fromUrl.indexOf('/', authorityStart);
        String authority = (pathStart < 0) ? fromUrl : fromUrl.substring(0, pathStart);
        if (location[0] == '/') {
            return authority + location;
        }
        if (pathStart < 0) {
            return authority + "/" + location;
        }
        int lastSlash = fromUrl.lastIndexOf('/');
        if (lastSlash < authority.length()) {
            return authority + "/" + location;
        }
        return fromUrl.substring(0, lastSlash + 1) + location;
    };

    auto doSinglePost = [&](const String &requestUrl) -> NetResponse {
        NetResponse r{};

        HTTPClient http;
        http.setConnectTimeout(timeoutMs);
        http.setTimeout(timeoutMs);

        http.setUserAgent("Mozilla/5.0 (Watchy; ESP32) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36");

        const char *headerKeys[] = {"Location", "Content-Type", "Content-Encoding"};
        http.collectHeaders(headerKeys, 3);

        const bool isHttps = requestUrl.startsWith("https://");
        bool began = false;
        WiFiClientSecure secureClient;
        if (isHttps) {
            secureClient.setInsecure();
            began = http.begin(secureClient, requestUrl);
        } else {
            began = http.begin(requestUrl);
        }

        if (!began) {
            r.httpCode = -1;
            r.error = "HTTP begin failed";
            return r;
        }

        http.addHeader("Accept-Encoding", "identity", false, true);
        http.addHeader("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
        http.addHeader("Connection", "close");
        http.addHeader("Content-Type", "application/x-www-form-urlencoded", false, true);
        if (acceptLanguage.length() > 0) {
            http.addHeader("Accept-Language", acceptLanguage, false, true);
        }
        if (origin.length() > 0) {
            http.addHeader("Origin", origin, false, true);
        }
        if (referer.length() > 0) {
            http.addHeader("Referer", referer, false, true);
        }

        const int code = http.POST(formBody);
        r.httpCode = code;
        r.location = http.header("Location");
        r.contentType = http.header("Content-Type");
        r.contentEncoding = http.header("Content-Encoding");

        if (code <= 0) {
            r.error = String("POST failed: ") + http.errorToString(code).c_str();
            http.end();
            return r;
        }

        if (code != HTTP_CODE_OK && !(code == HTTP_CODE_MOVED_PERMANENTLY || code == HTTP_CODE_FOUND ||
                                      code == HTTP_CODE_SEE_OTHER || code == HTTP_CODE_TEMPORARY_REDIRECT ||
                                      code == HTTP_CODE_PERMANENT_REDIRECT)) {
            r.error = String("HTTP error: ") + code;
            http.end();
            return r;
        }

        r.body = "";
        const int announcedSize = http.getSize();
        if (announcedSize > 0) {
            const size_t reserveBytes = (static_cast<size_t>(announcedSize) < kMaxBodyBytes)
                                            ? static_cast<size_t>(announcedSize)
                                            : kMaxBodyBytes;
            r.body.reserve(reserveBytes + 1);
        } else {
            r.body.reserve(2048);
        }

        WiFiClient *stream = http.getStreamPtr();
        int remaining = announcedSize;
        uint8_t buf[256];
        uint32_t lastDataMs = millis();

        bool usingTail = false;
        String head;
        std::unique_ptr<uint8_t[]> tailBuf;
        size_t tailPos = 0;
        size_t tailLen = 0;

        auto tailWrite = [&](const uint8_t *p, size_t n) {
            if (kTailBytes == 0) {
                return;
            }
            if (!tailBuf) {
                tailBuf.reset(new uint8_t[kTailBytes]);
            }
            for (size_t i = 0; i < n; ++i) {
                tailBuf[tailPos] = p[i];
                tailPos = (tailPos + 1) % kTailBytes;
                if (tailLen < kTailBytes) {
                    ++tailLen;
                }
            }
        };

        while ((http.connected() || stream->available() > 0) && (remaining > 0 || remaining == -1)) {
            const size_t avail = stream->available();
            if (avail == 0) {
                if (millis() - lastDataMs > timeoutMs) {
                    break;
                }
                delay(0);
                continue;
            }

            size_t toRead = avail;
            if (toRead > sizeof(buf)) {
                toRead = sizeof(buf);
            }
            if (remaining > 0 && toRead > static_cast<size_t>(remaining)) {
                toRead = static_cast<size_t>(remaining);
            }

            const size_t readCount = stream->readBytes(buf, toRead);
            if (readCount == 0) {
                delay(0);
                continue;
            }

            lastDataMs = millis();

            if (!usingTail) {
                if (r.body.length() + readCount <= kMaxBodyBytes) {
                    r.body.concat(reinterpret_cast<const char *>(buf), static_cast<unsigned int>(readCount));
                } else {
                    usingTail = true;
                    head = (r.body.length() > kHeadBytes) ? r.body.substring(0, kHeadBytes) : r.body;
                    if (r.body.length() > head.length()) {
                        tailWrite(reinterpret_cast<const uint8_t *>(r.body.c_str() + head.length()),
                                  static_cast<size_t>(r.body.length() - head.length()));
                    }
                    r.body = "";
                    tailWrite(buf, readCount);
                }
            } else {
                tailWrite(buf, readCount);
            }

            if (remaining > 0) {
                remaining -= static_cast<int>(readCount);
            }
        }

        if (usingTail) {
            String tailOut;
            if (tailBuf && tailLen > 0) {
                tailOut.reserve(tailLen + 1);
                const size_t start = (tailLen == kTailBytes) ? tailPos : 0;
                const size_t firstSeg = (start + tailLen <= kTailBytes) ? tailLen : (kTailBytes - start);
                tailOut.concat(reinterpret_cast<const char *>(tailBuf.get() + start), static_cast<unsigned int>(firstSeg));
                const size_t secondSeg = tailLen - firstSeg;
                if (secondSeg > 0) {
                    tailOut.concat(reinterpret_cast<const char *>(tailBuf.get()), static_cast<unsigned int>(secondSeg));
                }
            }

            r.body = head;
            if (tailOut.length() > 0) {
                r.body += "\n";
                r.body += tailOut;
            }
        }

        http.end();
        return r;
    };

    for (uint8_t redirects = 0; redirects <= kMaxRedirects; ++redirects) {
        NetResponse hopResp{};

        for (uint8_t attempt = 0; attempt < kMaxAttemptsPerHop; ++attempt) {
            hopResp = doSinglePost(currentUrl);
            if (hopResp.httpCode <= 0 && attempt + 1 < kMaxAttemptsPerHop) {
                delay(120);
                continue;
            }
            break;
        }

        resp = hopResp;

        if (resp.httpCode == HTTP_CODE_MOVED_PERMANENTLY || resp.httpCode == HTTP_CODE_FOUND ||
            resp.httpCode == HTTP_CODE_SEE_OTHER || resp.httpCode == HTTP_CODE_TEMPORARY_REDIRECT ||
            resp.httpCode == HTTP_CODE_PERMANENT_REDIRECT) {
            const String nextUrl = resolveRedirect(currentUrl, resp.location);
            if (nextUrl.length() == 0) {
                resp.error = String("Redirect without valid Location (HTTP ") + resp.httpCode + ")";
                break;
            }
            if (redirects == kMaxRedirects) {
                resp.error = "Too many redirects";
                break;
            }
            currentUrl = nextUrl;
            continue;
        }

        break;
    }

    url = currentUrl;

    if (shutdownRadio) {
        WiFi.mode(WIFI_OFF);
        btStop();
    }

    return resp;
}

bool parseIp(const String &ip, uint8_t out[4]) {
    int parts[4] = {0,0,0,0};
    if (sscanf(ip.c_str(), "%d.%d.%d.%d", &parts[0], &parts[1], &parts[2], &parts[3]) != 4) {
        return false;
    }
    for (int i = 0; i < 4; ++i) {
        if (parts[i] < 0 || parts[i] > 255) {
            return false;
        }
        out[i] = static_cast<uint8_t>(parts[i]);
    }
    return true;
}

namespace {
static char *sEditBuffer = nullptr;
static size_t sEditMaxLen = 0;
static size_t sEditLen = 0;
static size_t sEditCharIndex = 0;
static const char *sEditChars = nullptr;
static size_t sEditCharsCount = 0;
static volatile bool sEditDone = false;
static volatile bool sEditCancel = false;
static volatile bool sEditDirty = true;

static void editTargetBack(Watchy *watchy) {
    UiTemplates::waitForAllButtonsReleased(50, 100);
    if (sEditBuffer == nullptr || sEditMaxLen < 2) {
        sEditCancel = true;
        return;
    }
    if (sEditLen == 0) {
        sEditCancel = true;
        return;
    }
    sEditBuffer[--sEditLen] = '\0';
    sEditDirty = true;
}

static void editTargetUp(Watchy *watchy) {
    
    if (sEditCharsCount == 0) return;
    sEditCharIndex = (sEditCharIndex + 1) % sEditCharsCount;
    sEditDirty = true;
}

static void editTargetDown(Watchy *watchy) {
    
    if (sEditCharsCount == 0) return;
    sEditCharIndex = (sEditCharIndex + sEditCharsCount - 1) % sEditCharsCount;
    sEditDirty = true;
}

static void editTargetMenu(Watchy *watchy) {
    UiTemplates::waitForAllButtonsReleased(50, 100);
    if (sEditBuffer == nullptr || sEditChars == nullptr || sEditCharsCount == 0) {
        return;
    }

    const char c = sEditChars[sEditCharIndex];

    // Finish when pressing ACCEPT on space twice in a row (with a real target present).
    if (c == ' ' && sEditLen > 1 && sEditBuffer[sEditLen - 1] == ' ') {
        sEditBuffer[sEditLen - 1] = '\0';
        sEditDone = true;
        return;
    }

    if (sEditLen < sEditMaxLen - 1) {
        sEditBuffer[sEditLen++] = c;
        sEditBuffer[sEditLen] = '\0';
        sEditDirty = true;
    }
}
} // namespace

bool editTarget(Watchy &watchy, char *buffer, size_t maxLen, const char *title, const char *defaultValue) {
    gExitToMenuRequested = false;

    // Guard against tiny buffers; need room for at least 1 char + null.
    if (maxLen < 2) {
        return false;
    }

    // Allowed: space, lowercase, digits, dash, dot.
    static const char kChars[] = " abcdefghijklmnopqrstuvwxyz0123456789-.";
    const size_t kCharsCount = sizeof(kChars) - 1; // exclude null terminator

    sEditBuffer = buffer;
    sEditMaxLen = maxLen;
    sEditChars = kChars;
    sEditCharsCount = kCharsCount;

    sEditLen = strnlen(buffer, maxLen - 1);
    buffer[sEditLen] = '\0';

    if (sEditLen == 0 && defaultValue != nullptr && defaultValue[0] != '\0') {
        strncpy(buffer, defaultValue, maxLen - 1);
        buffer[maxLen - 1] = '\0';
        sEditLen = strnlen(buffer, maxLen - 1);
    }

    sEditDone = false;
    sEditCancel = false;
    sEditDirty = true;
    sEditCharIndex = 0;
    UiTemplates::waitForAllButtonsReleased();
    watchy.setButtonHandlers(editTargetBack, editTargetUp, editTargetMenu, editTargetDown);

    UIControlsRowLayout controls[4] = {
        {"BACK", &Watchy::backPressed},
        {"UP", &Watchy::upPressed},
        {"ADD", &Watchy::menuPressed},
        {"DOWN", &Watchy::downPressed},
    };

    while (!sEditDone && !sEditCancel) {
        UiSDK::renderControlsRow(watchy, controls);

        if (!sEditDirty) {
            delay(10);
            continue;
        }
        sEditDirty = false;

        UiSDK::initScreen(watchy.display);

        UITextSpec header{};
        header.x = 0;
        header.y = 36;
        header.font = UiSDK::defaultFont();
        header.text = title;

        // Show the tail of the target if it exceeds the view width.
        const size_t viewLen = 21;
        const char *startPtr = buffer;
        if (sEditLen > viewLen) {
            startPtr = buffer + (sEditLen - viewLen);
        }

        UITextSpec target{};
        target.x = 0;
        target.y = 60;
        target.font = UiSDK::defaultFont();
        target.text = (sEditLen == 0) ? "<empty>" : String(startPtr);

        const char currentChar = kChars[sEditCharIndex];
        const String currentLabel = (currentChar == ' ') ? String("[space]") : String(currentChar);

        UITextSpec caret{};
        caret.x = 0;
        caret.y = 82;
        caret.font = UiSDK::tinyMono6x8();
        caret.text = String("[a-z0-9.-]:") + currentLabel + "   SPACEx2 -> GO";

        UITextSpec specs[3] = {header, target, caret};

        UIAppSpec app{};
        app.texts = specs;
        app.textCount = 3;
        app.controls[0] = controls[0];
        app.controls[1] = controls[1];
        app.controls[2] = controls[2];
        app.controls[3] = controls[3];
        app.controlCount = 4;

        UiSDK::renderApp(watchy, app);
        watchy.display.display(true);
    }

    watchy.clearButtonHandlers();
    sEditBuffer = nullptr;
    sEditMaxLen = 0;
    sEditChars = nullptr;
    sEditCharsCount = 0;

    return sEditDone && !sEditCancel;
}

char* traceroute(const String &host, uint8_t maxHops, uint16_t timeoutMs) {
    static String buffer;
    buffer.remove(0);

    // Resolve host
    IPAddress target;
    if (!WiFi.hostByName(host.c_str(), target)) {
        buffer = "DNS failed for " + host;
        return const_cast<char*>(buffer.c_str());
    }

    int sock = lwip_socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sock < 0) {
        buffer = "ICMP socket error";
        return const_cast<char*>(buffer.c_str());
    }

    sockaddr_in dest{};
    dest.sin_family = AF_INET;
    dest.sin_port = 0;
    dest.sin_addr.s_addr = target;

    // Receive timeout
    timeval tv{};
    tv.tv_sec = timeoutMs / 1000;
    tv.tv_usec = (timeoutMs % 1000) * 1000;
    lwip_setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    const uint16_t ident = static_cast<uint16_t>(esp_random());
    uint8_t recvBuf[512];

    buffer += "Traceroute to ";
    buffer += target.toString();
    buffer += " (" + host + ")\n";

    for (uint8_t ttl = 1; ttl <= maxHops; ++ttl) {
        // Set TTL per probe
        lwip_setsockopt(sock, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl));

        IcmpEcho pkt{};
        pkt.type = 8; // echo request
        pkt.code = 0;
        pkt.identifier = lwip_htons(ident);
        pkt.sequence = lwip_htons(ttl);
        pkt.checksum = 0;
        pkt.checksum = icmpChecksum(&pkt, sizeof(pkt));

        const uint32_t sendMs = millis();
        int sent = lwip_sendto(sock, &pkt, sizeof(pkt), 0, reinterpret_cast<sockaddr*>(&dest), sizeof(dest));
        if (sent < 0) {
            buffer += String(ttl) + " send error\n";
            break;
        }

        sockaddr_in from{};
        socklen_t fromLen = sizeof(from);
        int recvd = lwip_recvfrom(sock, recvBuf, sizeof(recvBuf), 0, reinterpret_cast<sockaddr*>(&from), &fromLen);

        if (recvd < 0) {
            buffer += String(ttl) + " * * *\n";
            continue;
        }

        IPAddress hop(from.sin_addr.s_addr);

        const uint32_t rttMs = millis() - sendMs;
        uint8_t icmpType = 0xFF;
        uint8_t icmpCode = 0;
        if (recvd >= 1) {
            // Raw ICMP socket returns an IPv4 header followed by ICMP.
            const uint8_t ipVerIhl = recvBuf[0];
            const uint8_t ihlWords = (ipVerIhl & 0x0F);
            const uint16_t ipHdrLen = static_cast<uint16_t>(ihlWords) * 4;
            if (ipHdrLen >= 20 && ipHdrLen + 2 <= static_cast<uint16_t>(recvd)) {
                icmpType = recvBuf[ipHdrLen + 0];
                icmpCode = recvBuf[ipHdrLen + 1];
            }
        }

        buffer += String(ttl) + " " + hop.toString() + " " + String(rttMs) + "ms";
        if (icmpType != 0xFF) {
            buffer += " (icmp " + String(icmpType) + "/" + String(icmpCode) + ")";
        }
        buffer += "\n";

        // Destination reached when we get an echo reply (type 0) from target.
        if (hop == target && icmpType == 0) {
            buffer += "Reached destination\n";
            break;
        }
    }

    lwip_close(sock);
    return const_cast<char*>(buffer.c_str());
}

namespace {
static bool startsWithNoCase(const String &s, const char *prefix) {
    const size_t n = strlen(prefix);
    if (s.length() < n) return false;
    for (size_t i = 0; i < n; ++i) {
        char a = s[i];
        char b = prefix[i];
        if (a >= 'A' && a <= 'Z') a = static_cast<char>(a - 'A' + 'a');
        if (b >= 'A' && b <= 'Z') b = static_cast<char>(b - 'A' + 'a');
        if (a != b) return false;
    }
    return true;
}

static String trimCopy(String s) {
    s.trim();
    return s;
}

static String extractWhoisReferral(const String &ianaResp) {
    // IANA typically provides either:
    //   refer: whois.example
    //   whois: whois.example
    int start = 0;
    while (start >= 0 && start < static_cast<int>(ianaResp.length())) {
        int end = ianaResp.indexOf('\n', start);
        if (end < 0) end = ianaResp.length();
        String line = ianaResp.substring(start, end);
        line.trim();
        if (startsWithNoCase(line, "refer:")) {
            return trimCopy(line.substring(6));
        }
        if (startsWithNoCase(line, "whois:")) {
            return trimCopy(line.substring(6));
        }
        start = end + 1;
    }
    return String();
}

static bool tcpQueryText(const char *server,
                         uint16_t port,
                         const String &query,
                         uint32_t timeoutMs,
                         size_t maxBytes,
                         String &outText,
                         String &outError) {
    outText = "";
    outError = "";

    addrinfo hints{};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    addrinfo *res = nullptr;
    const String portStr = String(port);
    const int gai = getaddrinfo(server, portStr.c_str(), &hints, &res);
    if (gai != 0 || res == nullptr) {
        outError = String("DNS failed for ") + server;
        return false;
    }

    int sock = -1;
    for (addrinfo *p = res; p != nullptr; p = p->ai_next) {
        sock = lwip_socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sock < 0) continue;

        timeval tv{};
        tv.tv_sec = timeoutMs / 1000;
        tv.tv_usec = (timeoutMs % 1000) * 1000;
        lwip_setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        lwip_setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

        if (lwip_connect(sock, p->ai_addr, p->ai_addrlen) == 0) {
            break;
        }
        lwip_close(sock);
        sock = -1;
    }
    freeaddrinfo(res);

    if (sock < 0) {
        outError = String("Connect failed: ") + server;
        return false;
    }

    String q = query;
    q.replace("\r", "");
    if (!q.endsWith("\n")) {
        q += "\n";
    }
    // RFC 3912: requests terminated with CRLF.
    q.replace("\n", "\r\n");

    const char *qbuf = q.c_str();
    size_t qlen = q.length();
    while (qlen > 0) {
        const int sent = lwip_send(sock, qbuf, qlen, 0);
        if (sent <= 0) {
            outError = "Send failed";
            lwip_close(sock);
            return false;
        }
        qbuf += sent;
        qlen -= static_cast<size_t>(sent);
    }

    uint8_t buf[512];
    while (outText.length() < maxBytes) {
        const int n = lwip_recv(sock, buf, sizeof(buf), 0);
        if (n == 0) {
            break; // server closed -> end of response
        }
        if (n < 0) {
            break; // timeout or error
        }
        const size_t remaining = maxBytes - outText.length();
        const size_t toAppend = (static_cast<size_t>(n) > remaining) ? remaining : static_cast<size_t>(n);
        outText.concat(reinterpret_cast<const char *>(buf), static_cast<unsigned int>(toAppend));
    }

    lwip_close(sock);
    return true;
}

static uint16_t readU16(const uint8_t *msg, size_t len, size_t &off, bool &ok) {
    if (!ok || off + 2 > len) {
        ok = false;
        return 0;
    }
    const uint16_t v = (static_cast<uint16_t>(msg[off]) << 8) | msg[off + 1];
    off += 2;
    return v;
}

static uint32_t readU32(const uint8_t *msg, size_t len, size_t &off, bool &ok) {
    if (!ok || off + 4 > len) {
        ok = false;
        return 0;
    }
    const uint32_t v = (static_cast<uint32_t>(msg[off]) << 24) |
                       (static_cast<uint32_t>(msg[off + 1]) << 16) |
                       (static_cast<uint32_t>(msg[off + 2]) << 8) |
                       (static_cast<uint32_t>(msg[off + 3]));
    off += 4;
    return v;
}

static bool readDnsName(const uint8_t *msg,
                        size_t msgLen,
                        size_t &off,
                        String &outName) {
    outName = "";
    size_t pos = off;
    bool jumped = false;
    size_t jumpOff = 0;
    uint8_t depth = 0;

    while (pos < msgLen) {
        const uint8_t len = msg[pos];
        if (len == 0) {
            pos += 1;
            break;
        }

        // Pointer (compression)
        if ((len & 0xC0) == 0xC0) {
            if (pos + 1 >= msgLen) {
                return false;
            }
            const uint16_t ptr = (static_cast<uint16_t>(len & 0x3F) << 8) | msg[pos + 1];
            if (!jumped) {
                jumpOff = pos + 2;
                jumped = true;
            }
            if (ptr >= msgLen) {
                return false;
            }
            pos = ptr;
            if (++depth > 12) {
                return false;
            }
            continue;
        }

        // Label
        if ((len & 0xC0) != 0) {
            return false;
        }
        pos += 1;
        if (pos + len > msgLen) {
            return false;
        }
        if (outName.length() > 0) {
            outName += '.';
        }
        outName.concat(reinterpret_cast<const char *>(msg + pos), static_cast<unsigned int>(len));
        pos += len;
    }

    off = jumped ? jumpOff : pos;
    if (outName.length() == 0) {
        outName = ".";
    }
    return true;
}

static String formatIpv6(const uint8_t *p) {
    char buf[48];
    // Basic full form; no zero-compression to keep it simple.
    snprintf(buf, sizeof(buf),
             "%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",
             p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7],
             p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15]);
    return String(buf);
}

static String dnsTypeName(uint16_t t) {
    switch (t) {
        case 1: return "A";
        case 2: return "NS";
        case 5: return "CNAME";
        case 6: return "SOA";
        case 12: return "PTR";
        case 15: return "MX";
        case 16: return "TXT";
        case 28: return "AAAA";
        default: return String(t);
    }
}

static String buildPtrNameFromIpv4(const String &ipStr) {
    uint8_t oct[4] = {0, 0, 0, 0};
    if (sscanf(ipStr.c_str(), "%hhu.%hhu.%hhu.%hhu", &oct[0], &oct[1], &oct[2], &oct[3]) != 4) {
        return String();
    }
    return String(oct[3]) + "." + String(oct[2]) + "." + String(oct[1]) + "." + String(oct[0]) + ".in-addr.arpa";
}

static bool encodeQName(const String &name, uint8_t *out, size_t outCap, size_t &outLen) {
    outLen = 0;
    String n = name;
    n.trim();
    if (n.endsWith(".")) {
        n = n.substring(0, n.length() - 1);
    }
    if (n.length() == 0) {
        return false;
    }

    int start = 0;
    while (start < static_cast<int>(n.length())) {
        int dot = n.indexOf('.', start);
        if (dot < 0) {
            dot = n.length();
        }
        const int labLen = dot - start;
        if (labLen <= 0 || labLen > 63) {
            return false;
        }
        if (outLen + 1 + static_cast<size_t>(labLen) >= outCap) {
            return false;
        }
        out[outLen++] = static_cast<uint8_t>(labLen);
        for (int i = 0; i < labLen; ++i) {
            out[outLen++] = static_cast<uint8_t>(n[start + i]);
        }
        start = dot + 1;
    }
    if (outLen + 1 > outCap) {
        return false;
    }
    out[outLen++] = 0;
    return true;
}

static bool udpDnsExchange(IPAddress server,
                           const uint8_t *req,
                           size_t reqLen,
                           uint8_t *resp,
                           size_t respCap,
                           size_t &respLen,
                           uint32_t timeoutMs) {
    respLen = 0;
    int sock = lwip_socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) {
        return false;
    }

    timeval tv{};
    tv.tv_sec = timeoutMs / 1000;
    tv.tv_usec = (timeoutMs % 1000) * 1000;
    lwip_setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    sockaddr_in dst{};
    dst.sin_family = AF_INET;
    dst.sin_port = lwip_htons(53);
    dst.sin_addr.s_addr = static_cast<uint32_t>(server);

    const int sent = lwip_sendto(sock, req, reqLen, 0, reinterpret_cast<sockaddr *>(&dst), sizeof(dst));
    if (sent < 0) {
        lwip_close(sock);
        return false;
    }

    sockaddr_in from{};
    socklen_t fromLen = sizeof(from);
    const int n = lwip_recvfrom(sock, resp, respCap, 0, reinterpret_cast<sockaddr *>(&from), &fromLen);
    if (n > 0) {
        respLen = static_cast<size_t>(n);
    }

    lwip_close(sock);
    return respLen > 0;
}
} // namespace

String whoisLookup(const String &target, uint32_t timeoutMs, size_t maxBytes) {
    String result;

    if (target.length() == 0) {
        return "Empty target";
    }

    String ianaResp;
    String err;
    if (!tcpQueryText("whois.iana.org", 43, target, timeoutMs, maxBytes, ianaResp, err)) {
        return err.length() ? err : String("WHOIS failed");
    }

    const String referral = extractWhoisReferral(ianaResp);
    if (referral.length() == 0) {
        return ianaResp;
    }

    String referredResp;
    String err2;
    if (!tcpQueryText(referral.c_str(), 43, target, timeoutMs, maxBytes, referredResp, err2)) {
        // Fall back to IANA response, but keep the referral hint.
        result = "Referral: " + referral + "\n\n" + ianaResp;
        if (err2.length()) {
            result += "\n\n";
            result += "Referral query error: ";
            result += err2;
        }
        return result;
    }

    result = "Referral: " + referral + "\n\n" + referredResp;
    return result;
}

String dnsLookup(const String &name, uint16_t qtype, uint32_t timeoutMs, uint8_t attempts) {
    String n = name;
    n.trim();
    if (n.length() == 0) {
        return "Empty name";
    }

    // If looks like IPv4, default to PTR lookup.
    uint8_t ip4[4];
    if (parseIp(n, ip4)) {
        const String ptrName = buildPtrNameFromIpv4(n);
        if (ptrName.length() > 0) {
            n = ptrName;
            qtype = 12; // PTR
        }
    }

    IPAddress dnsServer = WiFi.dnsIP(0);
    if (dnsServer == IPAddress(0, 0, 0, 0)) {
        dnsServer = IPAddress(1, 1, 1, 1);
    }

    uint8_t req[512];
    size_t reqLen = 0;
    memset(req, 0, sizeof(req));

    const uint16_t id = static_cast<uint16_t>(esp_random());
    req[0] = static_cast<uint8_t>((id >> 8) & 0xFF);
    req[1] = static_cast<uint8_t>(id & 0xFF);
    // Flags: recursion desired.
    req[2] = 0x01;
    req[3] = 0x00;
    // QDCOUNT=1
    req[4] = 0x00;
    req[5] = 0x01;

    size_t off = 12;
    size_t qnameLen = 0;
    if (!encodeQName(n, req + off, sizeof(req) - off, qnameLen)) {
        return "Invalid name";
    }
    off += qnameLen;
    if (off + 4 > sizeof(req)) {
        return "Request too large";
    }
    req[off + 0] = static_cast<uint8_t>((qtype >> 8) & 0xFF);
    req[off + 1] = static_cast<uint8_t>(qtype & 0xFF);
    // QCLASS=IN
    req[off + 2] = 0x00;
    req[off + 3] = 0x01;
    off += 4;
    reqLen = off;

    uint8_t resp[512];
    size_t respLen = 0;

    bool got = false;
    for (uint8_t a = 0; a < attempts; ++a) {
        if (udpDnsExchange(dnsServer, req, reqLen, resp, sizeof(resp), respLen, timeoutMs)) {
            got = true;
            break;
        }
        delay(50);
    }
    if (!got) {
        return String("DNS timeout (server ") + dnsServer.toString() + ")";
    }

    if (respLen < 12) {
        return "DNS response too short";
    }
    const uint16_t rid = (static_cast<uint16_t>(resp[0]) << 8) | resp[1];
    if (rid != id) {
        // Not our response; report but still parse.
    }
    const uint16_t flags = (static_cast<uint16_t>(resp[2]) << 8) | resp[3];
    const uint8_t rcode = static_cast<uint8_t>(flags & 0x0F);
    const uint16_t qd = (static_cast<uint16_t>(resp[4]) << 8) | resp[5];
    const uint16_t an = (static_cast<uint16_t>(resp[6]) << 8) | resp[7];
    const uint16_t ns = (static_cast<uint16_t>(resp[8]) << 8) | resp[9];
    const uint16_t ar = (static_cast<uint16_t>(resp[10]) << 8) | resp[11];

    String out;
    out.reserve(2048);
    out += "Server: ";
    out += dnsServer.toString();
    out += "\n";
    out += "Query: ";
    out += n;
    out += " ";
    out += dnsTypeName(qtype);
    out += "\n\n";

    if (rcode != 0) {
        out += "RCODE: ";
        out += String(rcode);
        out += "\n\n";
    }

    bool ok = true;
    size_t roff = 12;

    // Skip questions
    for (uint16_t i = 0; i < qd; ++i) {
        String tmp;
        if (!readDnsName(resp, respLen, roff, tmp)) {
            return out + "Malformed question name";
        }
        (void)readU16(resp, respLen, roff, ok);
        (void)readU16(resp, respLen, roff, ok);
        if (!ok) {
            return out + "Malformed question";
        }
    }

    auto parseSection = [&](const char *title, uint16_t count) {
        if (count == 0) return;
        out += "[";
        out += title;
        out += "]\n";

        for (uint16_t i = 0; i < count; ++i) {
            String owner;
            if (!readDnsName(resp, respLen, roff, owner)) {
                out += "<bad name>\n";
                ok = false;
                return;
            }
            const uint16_t type = readU16(resp, respLen, roff, ok);
            const uint16_t klass = readU16(resp, respLen, roff, ok);
            const uint32_t ttl = readU32(resp, respLen, roff, ok);
            const uint16_t rdlen = readU16(resp, respLen, roff, ok);
            if (!ok || roff + rdlen > respLen) {
                out += "<bad rr>\n";
                ok = false;
                return;
            }

            out += owner;
            out += " ";
            out += String(ttl);
            out += " ";
            out += (klass == 1) ? "IN" : String(klass);
            out += " ";
            out += dnsTypeName(type);
            out += " ";

            const size_t rdataOff = roff;
            size_t tmpOff = rdataOff;

            if (type == 1 && rdlen == 4) {
                IPAddress ip(resp[rdataOff], resp[rdataOff + 1], resp[rdataOff + 2], resp[rdataOff + 3]);
                out += ip.toString();
            } else if (type == 28 && rdlen == 16) {
                out += formatIpv6(resp + rdataOff);
            } else if (type == 5 || type == 2 || type == 12) {
                String dn;
                if (readDnsName(resp, respLen, tmpOff, dn)) {
                    out += dn;
                } else {
                    out += "<bad name>";
                }
            } else if (type == 15 && rdlen >= 3) {
                const uint16_t pref = (static_cast<uint16_t>(resp[rdataOff]) << 8) | resp[rdataOff + 1];
                tmpOff = rdataOff + 2;
                String exch;
                if (readDnsName(resp, respLen, tmpOff, exch)) {
                    out += String(pref);
                    out += " ";
                    out += exch;
                } else {
                    out += String(pref);
                    out += " <bad name>";
                }
            } else if (type == 16 && rdlen >= 1) {
                // One or more character-strings
                size_t p = rdataOff;
                bool first = true;
                while (p < rdataOff + rdlen) {
                    const uint8_t l = resp[p++];
                    if (p + l > rdataOff + rdlen) break;
                    if (!first) out += " | ";
                    out += '"';
                    out.concat(reinterpret_cast<const char *>(resp + p), static_cast<unsigned int>(l));
                    out += '"';
                    p += l;
                    first = false;
                }
            } else if (type == 6) {
                String mname;
                String rname;
                if (!readDnsName(resp, respLen, tmpOff, mname) || !readDnsName(resp, respLen, tmpOff, rname)) {
                    out += "<bad soa>";
                } else {
                    bool ok2 = true;
                    const uint32_t serial = readU32(resp, respLen, tmpOff, ok2);
                    const uint32_t refresh = readU32(resp, respLen, tmpOff, ok2);
                    const uint32_t retry = readU32(resp, respLen, tmpOff, ok2);
                    const uint32_t expire = readU32(resp, respLen, tmpOff, ok2);
                    const uint32_t minimum = readU32(resp, respLen, tmpOff, ok2);
                    out += mname;
                    out += " ";
                    out += rname;
                    if (ok2) {
                        out += " "; out += String(serial);
                        out += " "; out += String(refresh);
                        out += " "; out += String(retry);
                        out += " "; out += String(expire);
                        out += " "; out += String(minimum);
                    }
                }
            } else {
                out += "[";
                out += String(rdlen);
                out += " bytes]";
            }

            out += "\n";
            roff += rdlen;
        }

        out += "\n";
    };

    parseSection("Answer", an);
    parseSection("Authority", ns);
    parseSection("Additional", ar);

    if (!ok) {
        out += "Parse error\n";
    }
    return out;
}
} // namespace NetUtils
