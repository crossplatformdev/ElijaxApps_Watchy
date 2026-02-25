#include "../watchy/Watchy.h"
#include "../sdk/UiSDK.h"
#include "../sdk/UiTemplates.h"
#include "../sdk/NetUtils.h"

#include <vector>

// Default Home URL (can be overridden via build flags or another header).
#ifndef HOME_URL
#define HOME_URL "example.com"
#endif

// Optional safety limit for huge pages. Set to 0 for uncapped (default).
#ifndef TEXTBROWSER_MAX_DOC_CHARS
#define TEXTBROWSER_MAX_DOC_CHARS 0
#endif

namespace {

static const uint8_t MAX_REDIRECTS = 5;

static const char *kTextBrowserHistoryNs = "hist_text";
static int8_t sTextBrowserHistorySelected = -1;

enum class BrowserState {
    LOADING,
    DISPLAYING,
    ERROR
};

enum class RequestMethod {
    GET,
    POST_FORM
};

struct BrowserRequest {
    RequestMethod method = RequestMethod::GET;
    String url;
    // For POST_FORM
    String formBody;
    String origin;
    String referer;
    String acceptLanguage;

    bool urlHadScheme() const {
        return url.startsWith("http://") || url.startsWith("https://");
    }
};

struct PageData {
    String url;
    String title;

    // Links discovered while parsing (uncapped; limited only by heap)
    std::vector<String> linkUrls;

    // The rendered page is a plain-text document with \n-separated lines.
    String plainText;
    // Per-line: -1 means not a link, otherwise index into linkUrls.
    std::vector<int16_t> lineLinkIndex;
    
    void clear() {
        url = "";
        title = "";
        plainText = "";
        linkUrls.clear();
        lineLinkIndex.clear();
    }

    uint16_t lineCount() const {
        return static_cast<uint16_t>(lineLinkIndex.size());
    }

    uint16_t linkCount() const {
        return static_cast<uint16_t>(linkUrls.size());
    }

    bool isLinkLine(uint16_t line) const {
        return (line < lineLinkIndex.size() && lineLinkIndex[line] >= 0);
    }

    int16_t linkIndexForLine(uint16_t line) const {
        if (line >= lineLinkIndex.size()) {
            return -1;
        }
        return lineLinkIndex[line];
    }
};

class UrlValidator {
public:
    static bool isHttpOrHttps(const String& url) {
        return url.startsWith("http://") || url.startsWith("https://");
    }
    
    static String normalize(const String& url) {
        if (url.startsWith("//")) {
            return String("https:") + url;
        }
        if (isHttpOrHttps(url)) {
            return url;
        }
        // Default to HTTPS; many modern sites no longer serve plain HTTP.
        return String("https://") + url;
    }
};

// The shared target editor only supports host-like strings (a-z0-9.- and space).
// For history and editing, we store/offer only the host portion.
static String editableHostFromUrl(String url) {
    url.trim();
    if (url.startsWith("http://")) {
        url = url.substring(7);
    } else if (url.startsWith("https://")) {
        url = url.substring(8);
    } else if (url.startsWith("//")) {
        url = url.substring(2);
    }
    const int slash = url.indexOf('/');
    if (slash >= 0) {
        url = url.substring(0, slash);
    }
    url.trim();
    return url;
}

static bool isRedirectStatus(int httpCode) {
    return httpCode == 301 || httpCode == 302 || httpCode == 303 || httpCode == 307 || httpCode == 308;
}

static bool startsWithIgnoreCaseAt(const String &s, uint32_t pos, const char *prefix) {
    uint32_t i = 0;
    while (prefix[i] != '\0') {
        if (pos + i >= s.length()) {
            return false;
        }
        char a = s.charAt(pos + i);
        char b = prefix[i];
        if (a >= 'A' && a <= 'Z') a = static_cast<char>(a - 'A' + 'a');
        if (b >= 'A' && b <= 'Z') b = static_cast<char>(b - 'A' + 'a');
        if (a != b) {
            return false;
        }
        ++i;
    }
    return true;
}

static String decodeEntities(String s) {
    s.replace("&nbsp;", " ");
    s.replace("&amp;", "&");
    s.replace("&lt;", "<");
    s.replace("&gt;", ">");
    s.replace("&quot;", "\"");
    s.replace("&#39;", "'");

    // Minimal numeric entity decode: &#NNN;
    int idx = 0;
    while (true) {
        idx = s.indexOf("&#", idx);
        if (idx < 0) break;
        int semi = s.indexOf(';', idx + 2);
        if (semi < 0) break;
        String num = s.substring(idx + 2, semi);
        bool ok = true;
        for (uint16_t i = 0; i < num.length(); ++i) {
            if (num[i] < '0' || num[i] > '9') {
                ok = false;
                break;
            }
        }
        if (!ok) {
            idx = semi + 1;
            continue;
        }
        const int code = num.toInt();
        if (code <= 0 || code > 255) {
            idx = semi + 1;
            continue;
        }
        s = s.substring(0, idx) + String(static_cast<char>(code)) + s.substring(semi + 1);
        idx += 1;
    }
    return s;
}

static String squeezeSpaces(const String &in) {
    String out;
    out.reserve(in.length());
    bool lastWasSpace = false;
    for (uint16_t i = 0; i < in.length(); ++i) {
        char c = in[i];
        if (c == '\r' || c == '\n' || c == '\t') {
            c = ' ';
        }
        if (c == ' ') {
            if (lastWasSpace) continue;
            lastWasSpace = true;
            out += ' ';
        } else {
            lastWasSpace = false;
            out += c;
        }
    }
    out.trim();
    return out;
}

static String extractAttrValue(const String &tag, const char *attrName) {
    // tag is the inside of <...> (no brackets)
    String lower = tag;
    lower.toLowerCase();
    String needle = String(attrName);
    needle.toLowerCase();
    int pos = lower.indexOf(needle);
    if (pos < 0) return "";
    pos = lower.indexOf('=', pos + needle.length());
    if (pos < 0) return "";
    pos++;
    while (pos < (int)tag.length() && (tag[pos] == ' ' || tag[pos] == '\t')) pos++;
    if (pos >= (int)tag.length()) return "";
    char quote = tag[pos];
    int start = pos;
    int end = -1;
    if (quote == '"' || quote == '\'') {
        start = pos + 1;
        end = tag.indexOf(quote, start);
    } else {
        // Unquoted attribute value
        start = pos;
        end = start;
        while (end < (int)tag.length()) {
            char c = tag[end];
            if (c == ' ' || c == '\t' || c == '>') break;
            end++;
        }
    }
    if (end < 0 || end <= start) return "";
    String v = tag.substring(start, end);
    v.trim();
    return decodeEntities(v);
}

static void appendLine(PageData &out, const String &line, int16_t linkIndex) {
#if TEXTBROWSER_MAX_DOC_CHARS > 0
    if (out.plainText.length() >= TEXTBROWSER_MAX_DOC_CHARS) {
        return;
    }
#endif
    if (out.plainText.length() > 0) {
        out.plainText += "\n";
    }
    out.plainText += line;
    out.lineLinkIndex.push_back(linkIndex);
}

static void appendWrapped(PageData &out, const String &text, uint8_t maxCharsPerLine) {
    String t = squeezeSpaces(decodeEntities(text));
    if (t.length() == 0) {
        return;
    }

    uint16_t start = 0;
    while (start < t.length()) {
        uint16_t end = start;
        uint16_t lastSpace = 0xFFFF;
        uint16_t count = 0;
        while (end < t.length() && count < maxCharsPerLine) {
            if (t[end] == ' ') lastSpace = end;
            end++;
            count++;
        }

        if (end < t.length() && lastSpace != 0xFFFF && lastSpace > start) {
            end = lastSpace;
        }

        String line = t.substring(start, end);
        line.trim();
        appendLine(out, line, -1);

        start = end;
        while (start < t.length() && t[start] == ' ') start++;
    }
}

static String stripTagsPreserveWhitespace(const String &html) {
    // Like stripTagsAndDecode, but preserves whitespace/newlines for <pre>.
    String out;
    out.reserve((html.length() < 256) ? html.length() : 256);
    bool inTag = false;
    for (uint32_t i = 0; i < html.length(); ++i) {
        const char c = html.charAt(i);
        if (c == '<') {
            inTag = true;
            continue;
        }
        if (c == '>') {
            inTag = false;
            continue;
        }
        if (inTag) continue;
        out += c;
    }
    out = decodeEntities(out);
    out.replace("\r", "");
    // Keep newlines; normalize tabs.
    out.replace("\t", "    ");
    return out;
}

static void appendPreformatted(PageData &out, const String &text, uint8_t maxCharsPerLine) {
    if (text.length() == 0) {
        return;
    }

    int start = 0;
    while (start <= (int)text.length()) {
        int end = text.indexOf('\n', start);
        if (end < 0) end = text.length();
        String line = text.substring(start, end);

        // Preserve spacing; wrap by character count.
        while (line.length() > maxCharsPerLine && maxCharsPerLine > 0) {
            appendLine(out, line.substring(0, maxCharsPerLine), -1);
            line = line.substring(maxCharsPerLine);
        }
        appendLine(out, line, -1);

        if (end >= (int)text.length()) break;
        start = end + 1;
    }
}

static void appendLinkLine(PageData &out, const String &label, const String &url, uint8_t maxCharsPerLine) {
    const int16_t linkIndex = static_cast<int16_t>(out.linkUrls.size());
    out.linkUrls.push_back(UrlValidator::normalize(url));

    // Render as gopher-like link lines.
    String line = String("=> ") + squeezeSpaces(label);
    line = decodeEntities(line);
    if (line.length() <= maxCharsPerLine) {
        appendLine(out, line, linkIndex);
        return;
    }

    // Wrap the label; only the first line is treated as the selectable link.
    uint16_t start = 0;
    bool first = true;
    while (start < line.length()) {
        uint16_t end = start;
        uint16_t lastSpace = 0xFFFF;
        uint16_t count = 0;
        while (end < line.length() && count < maxCharsPerLine) {
            if (line[end] == ' ') lastSpace = end;
            end++;
            count++;
        }
        if (end < line.length() && lastSpace != 0xFFFF && lastSpace > start) {
            end = lastSpace;
        }
        String piece = line.substring(start, end);
        piece.trim();
        appendLine(out, piece, first ? linkIndex : -1);
        first = false;
        start = end;
        while (start < line.length() && line[start] == ' ') start++;
    }
}

static String stripTagsAndDecode(const String &html) {
    String out;
    out.reserve((html.length() < 256) ? html.length() : 256);
    bool inTag = false;
    for (uint32_t i = 0; i < html.length(); ++i) {
        char c = html.charAt(i);
        if (c == '<') {
            inTag = true;
            continue;
        }
        if (c == '>') {
            inTag = false;
            continue;
        }
        if (inTag) continue;
        out += c;
    }
    out = decodeEntities(out);
    out = squeezeSpaces(out);
    return out;
}

static String trimCopy(String s) {
    s.replace('\r', ' ');
    s.replace('\n', ' ');
    s.trim();
    return s;
}

static String resolveRelativeUrl(const String& baseUrl, const String& maybeRelative) {
    if (maybeRelative.length() == 0) return "";

    // Already absolute or protocol-relative
    if (maybeRelative.startsWith("http://") || maybeRelative.startsWith("https://") || maybeRelative.startsWith("//")) {
        return UrlValidator::normalize(maybeRelative);
    }

    // Parse scheme://host from baseUrl
    int schemeSep = baseUrl.indexOf("://");
    if (schemeSep < 0) {
        return UrlValidator::normalize(maybeRelative);
    }
    int hostStart = schemeSep + 3;
    int pathStart = baseUrl.indexOf('/', hostStart);
    String origin = (pathStart >= 0) ? baseUrl.substring(0, pathStart) : baseUrl;

    if (maybeRelative.startsWith("/")) {
        return origin + maybeRelative;
    }

    // Relative to current directory
    String baseDir = baseUrl;
    int lastSlash = baseDir.lastIndexOf('/');
    if (lastSlash > hostStart) {
        baseDir = baseDir.substring(0, lastSlash + 1);
    } else if (pathStart < 0) {
        baseDir = origin + "/";
    } else {
        baseDir = origin + "/";
    }

    return baseDir + maybeRelative;
}

static bool tryExtractRedirectUrl(const NetResponse& resp, const String& currentUrl, String& outUrl) {
    outUrl = "";

    // 0) Best: Location header from HTTP response.
    if (resp.location.length() > 0) {
        outUrl = resolveRelativeUrl(currentUrl, trimCopy(resp.location));
        return outUrl.length() > 0;
    }

    // 1) Try to parse a Location header if NetUtils exposed headers in error/body.
    //    Accept variants like "Location: ...".
    String haystacks[2] = {resp.error, resp.body};
    for (uint8_t i = 0; i < 2; i++) {
        const String& s = haystacks[i];
        int loc = s.indexOf("Location:");
        if (loc < 0) loc = s.indexOf("location:");
        if (loc < 0) continue;

        int start = loc + 9;
        while (start < (int)s.length() && (s[start] == ' ' || s[start] == '\t')) start++;
        int end = s.indexOf('\n', start);
        if (end < 0) end = s.length();
        String candidate = s.substring(start, end);
        candidate = trimCopy(candidate);
        // Strip optional surrounding quotes
        if (candidate.startsWith("\"") && candidate.endsWith("\"")) {
            candidate = candidate.substring(1, candidate.length() - 1);
            candidate = trimCopy(candidate);
        }

        if (candidate.length() > 0) {
            outUrl = resolveRelativeUrl(currentUrl, candidate);
            return outUrl.length() > 0;
        }
    }

    // 2) Try HTML meta refresh redirects (common on some sites).
    //    Example: <meta http-equiv="refresh" content="0; url=https://example.com/">
    const String& html = resp.body;
    int refreshPos = html.indexOf("http-equiv=\"refresh\"");
    if (refreshPos < 0) refreshPos = html.indexOf("http-equiv='refresh'");
    if (refreshPos >= 0) {
        int contentPos = html.indexOf("content=", refreshPos);
        if (contentPos >= 0) {
            int quote1 = html.indexOf('"', contentPos);
            char quoteChar = '"';
            if (quote1 < 0) {
                quote1 = html.indexOf('\'', contentPos);
                quoteChar = '\'';
            }
            if (quote1 >= 0) {
                int quote2 = html.indexOf(quoteChar, quote1 + 1);
                if (quote2 > quote1) {
                    String content = html.substring(quote1 + 1, quote2);
                    int urlEq = content.indexOf("url=");
                    if (urlEq < 0) urlEq = content.indexOf("URL=");
                    if (urlEq >= 0) {
                        String candidate = content.substring(urlEq + 4);
                        candidate = trimCopy(candidate);
                        outUrl = resolveRelativeUrl(currentUrl, candidate);
                        return outUrl.length() > 0;
                    }
                }
            }
        }
    }

    return false;
}

class PageParser {
public:
    static void parse(const String& html, const String& baseUrl, PageData& out) {
        out.clear();
        out.url = baseUrl;

        const uint8_t maxCharsPerLine = static_cast<uint8_t>((WatchyDisplay::WIDTH - 12) / 6);
        extractTitle(html, out);
        buildPlainText(html, out, maxCharsPerLine);
    }

private:
    static void extractTitle(const String& html, PageData& out) {
        int titleStart = html.indexOf("<title>");
        int titleEnd = html.indexOf("</title>", titleStart + 7);
        
        if (titleStart >= 0 && titleEnd > titleStart) {
            out.title = html.substring(titleStart + 7, titleEnd);
            out.title.replace('\n', ' ');
            out.title.trim();
            if (out.title.length() > 30) {
                out.title = out.title.substring(0, 27) + "...";
            }
        }
    }

    static void buildPlainText(const String &html, PageData &out, uint8_t maxCharsPerLine) {
        // Header lines are part of the scrollable content (gopher-like).
        if (out.title.length() > 0) {
            appendWrapped(out, String("# ") + out.title, maxCharsPerLine);
        }
        appendWrapped(out, String("URL: ") + out.url, maxCharsPerLine);

        uint32_t i = 0;
        String textBuf;
        textBuf.reserve(128);

        auto flushText = [&]() {
            if (textBuf.length() > 0) {
                appendWrapped(out, textBuf, maxCharsPerLine);
                textBuf = "";
            }
        };

        while (i < html.length()) {
            // Avoid watchdog resets on large/complex pages by yielding periodically.
            if ((i & 0x3FF) == 0) {
                delay(0);
            }
            const char c = html.charAt(i);
            if (c != '<') {
                // Text node.
                char t = c;
                if (t == '\r' || t == '\n' || t == '\t') t = ' ';
                textBuf += t;
                ++i;
                continue;
            }

            // Tag start.
            const int tagEnd = html.indexOf('>', i + 1);
            if (tagEnd < 0) {
                break;
            }

            String tag = html.substring(i + 1, tagEnd);
            String tagLower = tag;
            tagLower.toLowerCase();
            tagLower.trim();

            // Handle script/style blocks: never display their contents.
            if (tagLower.startsWith("script") || tagLower.startsWith("style")) {
                flushText();
                const char *closeTag = tagLower.startsWith("script") ? "</script" : "</style";
                int closePos = html.indexOf(closeTag, tagEnd + 1);
                if (closePos < 0) {
                    i = tagEnd + 1;
                    continue;
                }
                int closeEnd = html.indexOf('>', closePos + 2);
                i = (closeEnd < 0) ? (closePos + 2) : (closeEnd + 1);
                continue;
            }

            // Preformatted blocks: preserve line breaks/spaces.
            if (tagLower == "pre" || tagLower.startsWith("pre ") || tagLower.startsWith("pre\t")) {
                flushText();
                const int textStart = tagEnd + 1;
                const int closePre = html.indexOf("</pre", textStart);
                if (closePre < 0) {
                    i = tagEnd + 1;
                    continue;
                }
                const String inner = html.substring(textStart, closePre);
                const String preText = stripTagsPreserveWhitespace(inner);
                appendPreformatted(out, preText, maxCharsPerLine);

                const int closeEnd = html.indexOf('>', closePre + 4);
                i = (closeEnd < 0) ? (closePre + 4) : (closeEnd + 1);
                continue;
            }

            // Inline tags that cause a line break: flush text so far, then skip tag.
            if (tagLower.startsWith("br") || tagLower.startsWith("hr")) {
                flushText();
                i = tagEnd + 1;
                continue;
            }

            // Block-ish tags -> line breaks.
            if (tagLower.startsWith("br") || 
                tagLower.startsWith("p") || tagLower.startsWith("/p") ||
                tagLower.startsWith("div") || tagLower.startsWith("/div") ||
                tagLower.startsWith("li") || tagLower.startsWith("/li") ||
                tagLower.startsWith("ul") || tagLower.startsWith("/ul") ||
                tagLower.startsWith("ol") || tagLower.startsWith("/ol") ||
                tagLower.startsWith("h1") || tagLower.startsWith("/h1") ||
                tagLower.startsWith("h2") || tagLower.startsWith("/h2") ||
                tagLower.startsWith("h3") || tagLower.startsWith("/h3") ||
                tagLower.startsWith("h4") || tagLower.startsWith("/h4") ||
                tagLower.startsWith("h5") || tagLower.startsWith("/h5") ||
                tagLower.startsWith("h6") || tagLower.startsWith("/h6") ||

                // HTML5 semantic/layout elements
                tagLower.startsWith("header") || tagLower.startsWith("/header") ||
                tagLower.startsWith("footer") || tagLower.startsWith("/footer") ||
                tagLower.startsWith("nav") || tagLower.startsWith("/nav") ||
                tagLower.startsWith("main") || tagLower.startsWith("/main") ||
                tagLower.startsWith("section") || tagLower.startsWith("/section") ||
                tagLower.startsWith("article") || tagLower.startsWith("/article") ||
                tagLower.startsWith("aside") || tagLower.startsWith("/aside") ||
                tagLower.startsWith("figure") || tagLower.startsWith("/figure") ||
                tagLower.startsWith("figcaption") || tagLower.startsWith("/figcaption") ||
                tagLower.startsWith("address") || tagLower.startsWith("/address") ||

                // Common content structure
                tagLower.startsWith("blockquote") || tagLower.startsWith("/blockquote") ||
                tagLower.startsWith("dl") || tagLower.startsWith("/dl") ||
                tagLower.startsWith("dt") || tagLower.startsWith("/dt") ||
                tagLower.startsWith("dd") || tagLower.startsWith("/dd") ||

                // Table-ish structure (simple)
                tagLower.startsWith("table") || tagLower.startsWith("/table") ||
                tagLower.startsWith("thead") || tagLower.startsWith("/thead") ||
                tagLower.startsWith("tbody") || tagLower.startsWith("/tbody") ||
                tagLower.startsWith("tfoot") || tagLower.startsWith("/tfoot") ||
                tagLower.startsWith("tr") || tagLower.startsWith("/tr") ||
                tagLower.startsWith("td") || tagLower.startsWith("/td") ||
                tagLower.startsWith("th") || tagLower.startsWith("/th") ||
                
                tagLower.startsWith("hr")) {
                flushText();
                i = tagEnd + 1;
                continue;
            }

            // Images: show alt text.
            if (tagLower.startsWith("img")) {
                flushText();
                String alt = extractAttrValue(tag, "alt");
                alt = squeezeSpaces(alt);
                if (alt.length() > 0) {
                    appendWrapped(out, String("[img] ") + alt, maxCharsPerLine);
                }
                i = tagEnd + 1;
                continue;
            }

            // Links: turn them into selectable gopher-style lines.
            if (tagLower.startsWith("a ") || tagLower == "a" || tagLower.startsWith("a\t")) {
                flushText();
                String href = extractAttrValue(tag, "href");
                String resolved = resolveRelativeUrl(out.url, href);

                int textStart = tagEnd + 1;
                int closeA = html.indexOf("</a", textStart);
                if (closeA < 0) {
                    i = tagEnd + 1;
                    continue;
                }
                String inner = html.substring(textStart, closeA);
                String label = stripTagsAndDecode(inner);
                if (label.length() == 0) {
                    label = resolved.length() ? resolved : href;
                }

                if (resolved.length() > 0) {
                    appendLinkLine(out, label, resolved, maxCharsPerLine);
                } else {
                    appendWrapped(out, label, maxCharsPerLine);
                }

                int closeEnd = html.indexOf('>', closeA + 3);
                i = (closeEnd < 0) ? (closeA + 3) : (closeEnd + 1);
                continue;
            }

            // Default: ignore tag.
            i = tagEnd + 1;
        }

        flushText();
        if (out.lineCount() == 0) {
            appendWrapped(out, "(empty)", maxCharsPerLine);
        }
    }
};

class BrowserHistory {
private:
    std::vector<BrowserRequest> history;
    
public:
    BrowserHistory() = default;
    
    void push(const BrowserRequest& req) {
        history.push_back(req);
    }
    
    BrowserRequest pop() {
        if (history.empty()) {
            return BrowserRequest{};
        }
        BrowserRequest req = history.back();
        history.pop_back();
        return req;
    }
    
    bool isEmpty() const {
        return history.empty();
    }
    
    void clear() {
        history.clear();
    }
};

static NetResponse fetchRequest(Watchy &watchy,
                               const BrowserRequest &req,
                               String &inOutUrl,
                               bool allowHttpFallback,
                               bool startUrlHadScheme) {
    NetResponse resp{};
    String fetchUrl = inOutUrl;

    if (req.method == RequestMethod::POST_FORM) {
        resp = NetUtils::httpPostForm(fetchUrl,
                                      req.formBody,
                                      req.origin,
                                      req.referer,
                                      req.acceptLanguage,
                                      30000,
                                      false);
    } else {
        resp = NetUtils::httpGet(fetchUrl, 30000, false);
    }

    if (allowHttpFallback && resp.httpCode != 200 && !startUrlHadScheme && inOutUrl.startsWith("https://")) {
        const int httpsPrefixLen = 8; // strlen("https://")
        String httpFallback = String("http://") + inOutUrl.substring(httpsPrefixLen);
        fetchUrl = httpFallback;
        if (req.method == RequestMethod::POST_FORM) {
            resp = NetUtils::httpPostForm(fetchUrl,
                                          req.formBody,
                                          req.origin,
                                          req.referer,
                                          req.acceptLanguage,
                                          30000,
                                          false);
        } else {
            resp = NetUtils::httpGet(fetchUrl, 30000, false);
        }
    }

    inOutUrl = fetchUrl;
    return resp;
}

} // namespace

namespace {
static volatile bool sTbBackEvent = false;
static volatile bool sTbUpEvent = false;
static volatile bool sTbMenuEvent = false;
static volatile bool sTbDownEvent = false;

static void tbBack(Watchy *watchy) {
    UiTemplates::waitForAllButtonsReleased(50, 100);
    sTbBackEvent = true;
}
static void tbUp(Watchy *watchy) {
    UiTemplates::waitForAllButtonsReleased(50, 100);                                                                
    sTbUpEvent = true;
}
static void tbMenu(Watchy *watchy) {
    UiTemplates::waitForAllButtonsReleased(50, 100);                                                                                                                                
    sTbMenuEvent = true;
}
static void tbDown(Watchy *watchy) {
    UiTemplates::waitForAllButtonsReleased(50, 100);
    sTbDownEvent = true;
}
} // namespace

static void runTextBrowser(Watchy &watchy, const BrowserRequest &startReq) {
    guiState = APP_STATE;
    UiTemplates::waitForAllButtonsReleased();
    watchy.setButtonHandlers(tbBack, tbUp, tbMenu, tbDown);
    sTbBackEvent = sTbUpEvent = sTbMenuEvent = sTbDownEvent = false;

    BrowserHistory browserHistory;
    PageData currentPage;
    BrowserState state = BrowserState::LOADING;

    BrowserRequest currentReq = startReq;
    String targetUrl = UrlValidator::normalize(currentReq.url);
    bool startUrlHadScheme = currentReq.urlHadScheme();
    String errorMessage = "";

    uint16_t firstLine = 0;
    uint16_t selectedLine = 0;
    uint8_t visibleLines = 10;
    uint16_t computedLineHeight = 10;
    bool needsRefresh = true;

    uint32_t backHeldSinceMs = 0;
    bool backHoldTriggered = false;
    const uint32_t kBackHoldExitMs = 900;

    uint32_t upLastRepeatMs = 0;
    uint32_t downLastRepeatMs = 0;
    const uint32_t kScrollRepeatMs = 10;

    auto readStableUpDown = [&]() {
        bool up1 = (digitalRead(UP_BTN_PIN) == ACTIVE_LOW);
        bool down1 = (digitalRead(DOWN_BTN_PIN) == ACTIVE_LOW);
        if (up1 != down1) {
            delay(2);
            bool up2 = (digitalRead(UP_BTN_PIN) == ACTIVE_LOW);
            bool down2 = (digitalRead(DOWN_BTN_PIN) == ACTIVE_LOW);
            if (up2 != down2) {
                up1 = up2;
                down1 = down2;
            }
        }
        return std::pair<bool, bool>(up1, down1);
    };

    auto computeVisibleLines = [&]() -> uint8_t {
        const int16_t viewH = WatchyDisplay::HEIGHT - 36;
        const int16_t padding = 6;
        const int16_t lineHeight = 10;
        const int16_t contentH = viewH - padding * 2;
        uint8_t v = (contentH > 0) ? static_cast<uint8_t>(contentH / lineHeight) : 1;
        if (v < 1) v = 1;
        return v;
    };

    auto initSelectionForPage = [&]() {
        const uint16_t totalLines = currentPage.lineCount();
        visibleLines = computeVisibleLines();

        firstLine = 0;
        if (totalLines == 0) {
            selectedLine = 0;
            return;
        }

        // Requirement: select first link if it appears within the visible lines; otherwise
        // select the last visible line.
        const uint16_t scanMax = (totalLines < visibleLines) ? totalLines : visibleLines;
        uint16_t candidate = (scanMax > 0) ? (scanMax - 1) : 0;
        for (uint16_t i = 0; i < scanMax; ++i) {
            if (currentPage.isLinkLine(i)) {
                candidate = i;
                break;
            }
        }

        selectedLine = candidate;
    };
    
    while (true) {
        UIControlsRowLayout controls[4] = {
            {"BACK", &Watchy::backPressed},
            {"UP", &Watchy::upPressed},
            {"GO", &Watchy::menuPressed},
            {"DOWN", &Watchy::downPressed},
        };

        // Pump button input frequently.
        UiSDK::renderControlsRow(watchy, controls);

        // Long-press BACK exits immediately (browser apps requirement).
        const bool backDown = (digitalRead(BACK_BTN_PIN) == ACTIVE_LOW);
        if (backDown) {
            if (backHeldSinceMs == 0) {
                backHeldSinceMs = millis();
                backHoldTriggered = false;
            } else if (!backHoldTriggered && (millis() - backHeldSinceMs >= kBackHoldExitMs)) {
                backHoldTriggered = true;
                watchy.showMenu(menuIndex);
                return;
            }
        } else {
            backHeldSinceMs = 0;
            backHoldTriggered = false;
        }

        // UP/DOWN scrolling must not be debounced; repeat every 10ms while held.
        const auto upDownPair = readStableUpDown();
        const bool upDown = upDownPair.first;
        const bool downDown = upDownPair.second;
        const uint32_t nowMs = millis();

        // Discrete UP/DOWN presses (from callbacks) should not double-step with the
        // hold-repeat logic below.
        if (sTbUpEvent) {
            sTbUpEvent = false;
            upLastRepeatMs = nowMs;
            if (selectedLine > 0) {
                selectedLine--;
                if (selectedLine < firstLine) {
                    firstLine = selectedLine;
                }
                needsRefresh = true;
            }
        }
        if (sTbDownEvent) {
            sTbDownEvent = false;
            downLastRepeatMs = nowMs;
            const uint16_t totalLines = currentPage.lineCount();
            if (totalLines > 0 && selectedLine + 1 < totalLines) {
                selectedLine++;
                if (selectedLine >= static_cast<uint32_t>(firstLine + visibleLines)) {
                    firstLine = static_cast<uint16_t>(selectedLine - visibleLines + 1);
                }
                needsRefresh = true;
            }
        }

        if (!upDown) {
            upLastRepeatMs = 0;
        }
        if (!downDown) {
            downLastRepeatMs = 0;
        }

        if (upDown) {
            if (upLastRepeatMs == 0 || (nowMs - upLastRepeatMs >= kScrollRepeatMs)) {
                upLastRepeatMs = nowMs;
                if (selectedLine > 0) {
                    selectedLine--;
                    if (selectedLine < firstLine) {
                        firstLine = selectedLine;
                    }
                    needsRefresh = true;
                }
            }
        }

        if (downDown) {
            if (downLastRepeatMs == 0 || (nowMs - downLastRepeatMs >= kScrollRepeatMs)) {
                downLastRepeatMs = nowMs;
                const uint16_t totalLines = currentPage.lineCount();
                if (totalLines > 0 && selectedLine + 1 < totalLines) {
                    selectedLine++;
                    if (selectedLine >= static_cast<uint32_t>(firstLine + visibleLines)) {
                        firstLine = static_cast<uint16_t>(selectedLine - visibleLines + 1);
                    }
                    needsRefresh = true;
                }
            }
        }

        if (needsRefresh) {
            needsRefresh = false;
            
            if (state == BrowserState::LOADING) {
                UiSDK::initScreen(watchy.display);

                UIScrollableTextSpec loading{};
                loading.x = 0;
                loading.y = 18;
                loading.w = WatchyDisplay::WIDTH;
                loading.h = WatchyDisplay::HEIGHT - 36;
                loading.font = UiSDK::tinyMono6x8();
                loading.fillBackground = false;
                loading.textRef = nullptr;
                loading.text = (currentReq.method == RequestMethod::POST_FORM)
                                   ? (String("Loading (POST)...\n") + targetUrl)
                                   : (String("Loading...\n") + targetUrl);
                loading.firstLine = 0;
                loading.maxLines = 6;
                loading.lineHeight = 10;
                loading.centered = false;

                UIAppSpec app{};
                app.scrollTexts = &loading;
                app.scrollTextCount = 1;
                app.controls[0] = controls[0];
                app.controls[1] = controls[1];
                app.controls[2] = controls[2];
                app.controls[3] = controls[3];
                UiSDK::renderApp(watchy, app);
                
                // Attempt to load page
                if (!NetUtils::ensureWiFi(watchy, 1, 6000)) {
                    errorMessage = "WiFi connection failed";
                    state = BrowserState::ERROR;
                    needsRefresh = true;
                    continue;
                }

                // Fetch once; NetUtils::{httpGet,httpPostForm} follow redirects internally and
                // update fetchUrl to the final URL.
                String fetchUrl = targetUrl;
                NetResponse resp = fetchRequest(watchy, currentReq, fetchUrl, true, startUrlHadScheme);

                // Power: shut down radio once after attempts finish.
                WiFi.mode(WIFI_OFF);
                btStop();

                if (resp.httpCode == 200) {
                    // If we still got a compressed payload, we can't decode it on-device.
                    // Show a clear error instead of a blank/garbled page.
                    if (resp.contentEncoding.length() > 0 && resp.contentEncoding != "identity") {
                        errorMessage = String("Unsupported Content-Encoding: ") + resp.contentEncoding;
                        if (resp.contentType.length() > 0) {
                            errorMessage += String("\nType: ") + resp.contentType;
                        }
                        state = BrowserState::ERROR;
                        currentPage.clear();
                        currentPage.url = targetUrl;
                        currentPage.title = "Error";
                        const uint8_t maxCharsPerLine = static_cast<uint8_t>((WatchyDisplay::WIDTH - 12) / 6);
                        appendWrapped(currentPage, String("Error: ") + errorMessage, maxCharsPerLine);
                        appendWrapped(currentPage, "Press GO to edit URL.", maxCharsPerLine);
                        appendWrapped(currentPage, "Press BACK to exit.", maxCharsPerLine);
                        initSelectionForPage();
                        needsRefresh = true;
                        continue;
                    }

                    // Ensure the displayed URL matches the final destination.
                    targetUrl = fetchUrl;
                    PageParser::parse(resp.body, targetUrl, currentPage);
                    state = BrowserState::DISPLAYING;
                    initSelectionForPage();
                } else {
                    errorMessage = (resp.error.length() > 0) ? resp.error : ("HTTP Error: " + String(resp.httpCode));
                    if (isRedirectStatus(resp.httpCode)) {
                        errorMessage += " (redirect)";
                    }
                    state = BrowserState::ERROR;
                    currentPage.clear();
                    currentPage.url = targetUrl;
                    currentPage.title = "Error";
                    const uint8_t maxCharsPerLine = static_cast<uint8_t>((WatchyDisplay::WIDTH - 12) / 6);
                    appendWrapped(currentPage, String("Error: ") + errorMessage, maxCharsPerLine);
                    appendWrapped(currentPage, "Press GO to edit URL.", maxCharsPerLine);
                    appendWrapped(currentPage, "Press BACK to exit.", maxCharsPerLine);
                    initSelectionForPage();
                }
                needsRefresh = true;
                continue;
            }
            
            // Render current state: full-screen scrollable text + optional link highlight overlay.
            UiSDK::initScreen(watchy.display);
            UIScrollableTextSpec view{};
            view.x = 0;
            view.y = 18;
            view.w = WatchyDisplay::WIDTH;
            view.h = WatchyDisplay::HEIGHT - 36;
            view.font = UiSDK::tinyMono6x8();
            view.fillBackground = false;
            view.textRef = &currentPage.plainText;
            view.firstLine = firstLine;
            view.maxLines = 200;
            view.lineHeight = 10;
            view.centered = false;

            // Compute visible line count similar to UiSDK padding.
            computedLineHeight = 10;
            const int16_t padding = 6;
            const int16_t contentH = view.h - padding * 2;
            visibleLines = (contentH > 0) ? static_cast<uint8_t>(contentH / computedLineHeight) : 1;
            if (visibleLines < 1) visibleLines = 1;

            // Render only what can be visible; the document itwatchy can be arbitrarily long.
            view.maxLines = visibleLines;

            UIAppSpec app{};
            app.scrollTexts = &view;
            app.scrollTextCount = 1;
            app.controls[0] = controls[0];
            app.controls[1] = controls[1];
            app.controls[2] = controls[2];
            app.controls[3] = controls[3];
            UiSDK::renderApp(watchy, app);

            // Highlight current line if it's a link.
            if (currentPage.isLinkLine(selectedLine) && selectedLine >= firstLine && selectedLine < static_cast<uint32_t>(firstLine + visibleLines)) {
                const uint16_t bg = UiSDK::getWatchfaceBg(BASE_POLARITY);
                const uint16_t fg = UiSDK::getWatchfaceFg(BASE_POLARITY);
                const int16_t contentY = view.y + padding;
                const int16_t contentX = view.x + padding;

                const int16_t highlightY = static_cast<int16_t>(contentY + (selectedLine - firstLine) * computedLineHeight);

                watchy.display.fillRect(view.x, highlightY, view.w, computedLineHeight, fg);
                // Redraw the current line inverted over the highlight bar.
                // Extract the line text by scanning the plainText.
                uint16_t lineNo = 0;
                int start = 0;
                int end = -1;
                while (lineNo < selectedLine) {
                    end = currentPage.plainText.indexOf('\n', start);
                    if (end < 0) {
                        start = currentPage.plainText.length();
                        break;
                    }
                    start = end + 1;
                    lineNo++;
                }
                end = currentPage.plainText.indexOf('\n', start);
                if (end < 0) end = currentPage.plainText.length();
                String lineText = currentPage.plainText.substring(start, end);
                UiSDK::drawTextLine(watchy.display, contentX, highlightY, lineText, view.font, bg);
            }

            // Overlay updates require an additional partial refresh.
            watchy.display.display(true);
        }
        
        // Handle input
        if (sTbBackEvent) {
            sTbBackEvent = false;
            if (!browserHistory.isEmpty()) {
                currentReq = browserHistory.pop();
                targetUrl = UrlValidator::normalize(currentReq.url);
                startUrlHadScheme = currentReq.urlHadScheme();
                state = BrowserState::LOADING;
                needsRefresh = true;
                continue;
            }
            watchy.clearButtonHandlers();
            watchy.showMenu(menuIndex);
            return;
        }

        // UP/DOWN handled above as non-debounced held keys.

        if (sTbMenuEvent) {
            sTbMenuEvent = false;
            // GO: open current link if present; otherwise edit URL.
            const int16_t linkIdx = currentPage.linkIndexForLine(selectedLine);
            if (linkIdx >= 0 && static_cast<size_t>(linkIdx) < currentPage.linkUrls.size()) {
                    browserHistory.push(currentReq);
                    currentReq = BrowserRequest{};
                    currentReq.method = RequestMethod::GET;
                    currentReq.url = UrlValidator::normalize(currentPage.linkUrls[static_cast<size_t>(linkIdx)]);
                    targetUrl = UrlValidator::normalize(currentReq.url);
                    startUrlHadScheme = currentReq.urlHadScheme();
                    state = BrowserState::LOADING;
                    needsRefresh = true;
                    continue;
            }

            // URL edit uses shared SDK history (host-only) and persists only if changed.
            char host[256] = {0};
            const String currentHost = editableHostFromUrl(targetUrl);
            strncpy(host, currentHost.c_str(), sizeof(host) - 1);

            UiTemplates::HistoryPickerSpec picker;
            picker.title = "Browse";
            picker.nvsNamespace = kTextBrowserHistoryNs;
            picker.exampleValue = HOME_URL;
            picker.newLabel = "New URL";
            picker.maxEntries = UiTemplates::HISTORY_MAX_ENTRIES;
            picker.maxEntryLen = UiTemplates::HISTORY_MAX_ENTRY_LEN;

            UiTemplates::HistoryPickKind pickKind = UiTemplates::HistoryPickKind::NewTarget;
            if (!UiTemplates::runHistoryPickerNvs(watchy, picker, host, sizeof(host), pickKind, sTextBrowserHistorySelected)) {
                watchy.clearButtonHandlers();
                watchy.showMenu(menuIndex);
                return;
            }

            if (pickKind == UiTemplates::HistoryPickKind::NewTarget && host[0] == '\0' && currentHost.length() > 0) {
                strncpy(host, currentHost.c_str(), sizeof(host) - 1);
                host[sizeof(host) - 1] = '\0';
            }

            const String originalTarget = String(host);
            if (NetUtils::editTarget(watchy, host, sizeof(host), "Browse URL")) {
                if (host[0] != '\0' && String(host) != originalTarget) {
                    UiTemplates::historyAddUniqueNvs(picker.nvsNamespace, host, picker.maxEntries, picker.maxEntryLen);
                }

                if (state == BrowserState::DISPLAYING) {
                    browserHistory.push(currentReq);
                }
                currentReq = BrowserRequest{};
                currentReq.method = RequestMethod::GET;
                currentReq.url = UrlValidator::normalize(String(host));
                targetUrl = UrlValidator::normalize(currentReq.url);
                startUrlHadScheme = currentReq.urlHadScheme();
                state = BrowserState::LOADING;
                needsRefresh = true;
            } else if (NetUtils::consumeExitToMenuRequest()) {
                watchy.showMenu(menuIndex);
                return;
            }
        }

        // Keep scroll position in bounds if the page changes.
        const uint16_t totalLines = currentPage.lineCount();
        if (totalLines > 0) {
            if (selectedLine >= totalLines) {
                selectedLine = totalLines - 1;
                needsRefresh = true;
            }
            if (selectedLine < firstLine) {
                firstLine = selectedLine;
                needsRefresh = true;
            }
            if (selectedLine >= static_cast<uint32_t>(firstLine + visibleLines)) {
                firstLine = static_cast<uint16_t>(selectedLine - visibleLines + 1);
                needsRefresh = true;
            }
        }
    }
}

void Watchy::showTextBrowser(const String& startUrl) {
    BrowserRequest req{};
    req.method = RequestMethod::GET;
    req.url = startUrl;
    runTextBrowser(*this, req);
}

void Watchy::showTextBrowserPostForm(const String &url,
                                     const String &formBody,
                                     const String &origin,
                                     const String &referer,
                                     const String &acceptLanguage) {
    BrowserRequest req{};
    req.method = RequestMethod::POST_FORM;
    req.url = url;
    req.formBody = formBody;
    req.origin = origin;
    req.referer = referer;
    req.acceptLanguage = acceptLanguage;
    runTextBrowser(*this, req);
}

void Watchy::showTextBrowserHome() {
    guiState = APP_STATE;

    char host[256] = {0};
    UiTemplates::HistoryPickerSpec picker;
    picker.title = "Browse";
    picker.nvsNamespace = kTextBrowserHistoryNs;
    picker.exampleValue = HOME_URL;
    picker.newLabel = "New URL";
    picker.maxEntries = UiTemplates::HISTORY_MAX_ENTRIES;
    picker.maxEntryLen = UiTemplates::HISTORY_MAX_ENTRY_LEN;

    UiTemplates::HistoryPickKind pickKind = UiTemplates::HistoryPickKind::NewTarget;
    if (!UiTemplates::runHistoryPickerNvs(*this, picker, host, sizeof(host), pickKind, sTextBrowserHistorySelected)) {
        showMenu(menuIndex);
        return;
    }

    const String originalTarget = String(host);
    if (!NetUtils::editTarget(*this, host, sizeof(host), "Browse URL")) {
        showMenu(menuIndex);
        return;
    }

    if (host[0] != '\0' && String(host) != originalTarget) {
        UiTemplates::historyAddUniqueNvs(picker.nvsNamespace, host, picker.maxEntries, picker.maxEntryLen);
    }

    showTextBrowser(UrlValidator::normalize(String(host)));
}
