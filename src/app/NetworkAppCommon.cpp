
#include "WatchyUi.h"
#include "NetworkAppCommon.h"
#include "AppDisplay.h"
#include "WatchyPowerDiagnostics.h"
#include "Watchy.h"

#include <WiFiClientSecure.h>
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <lwip/inet.h>
#include <lwip/sockets.h>
#include <new>

namespace NetworkApps {
namespace {

constexpr uint8_t documentLinesPerPage = 20;
constexpr uint8_t linkRowsPerPage = 5;
constexpr uint8_t feedRowsPerPage = 4;
constexpr uint8_t feedTitleLines = 4;
constexpr uint8_t feedTitleColumns = 31;
constexpr size_t maximumFeedTitleLength =
  feedTitleLines * feedTitleColumns;
constexpr size_t maximumFeedContentLength = 1024;
constexpr size_t maximumFeedUrlLength = 512;
constexpr uint32_t longPressMs = 700;
constexpr size_t maximumTextBytes = 12288;
constexpr size_t networkHeapReserve = 49152;
constexpr size_t minimumDownloadBytes = 1024;
constexpr uint8_t gopherRowsPerView = 10;
constexpr int16_t gopherContentTop = 21;
constexpr uint8_t gopherTextColumns = 32;
constexpr uint8_t maximumGopherItems = 72;
constexpr uint8_t maximumGopherTargets = 12;
constexpr size_t gopherLabelSize = gopherTextColumns + 1;
constexpr size_t gopherTargetStorageSize =
  maximumGopherTargets * (maximumUrlLength + 1);
constexpr uint8_t noGopherTarget = UINT8_MAX;
static_assert(gopherTargetStorageSize <= UINT16_MAX,
        "GOPHER target offsets must fit in uint16_t");
constexpr char editorCharacters[] =
    " abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"
    ".-_/:?&=%+#@";

class CappedStream : public Stream {
public:
  CappedStream(String &output, size_t limit)
      : data(output), maximumSize(limit) {
    data = "";
    reserved = data.reserve(limit + 1);
  }

  int available() override { return 0; }
  int read() override { return -1; }
  int peek() override { return -1; }
  void flush() override {}

  size_t write(uint8_t value) override {
    return write(&value, 1);
  }

  size_t write(const uint8_t *buffer, size_t size) override {
    size_t room = data.length() < maximumSize
                      ? maximumSize - data.length()
                      : 0;
    size_t accepted = std::min(room, size);
    if (accepted > 0 && !data.concat(buffer, accepted)) {
      allocationFailed = true;
      return 0;
    }
    overflowed = overflowed || accepted != size;
    return accepted;
  }

  bool ready() const { return reserved; }
  bool failed() const { return allocationFailed; }
  bool overflowed = false;

private:
  String &data;
  size_t maximumSize;
  bool reserved = false;
  bool allocationFailed = false;
};

enum GopherItemType : uint8_t {
  GOPHER_TEXT,
  GOPHER_HEADING,
  GOPHER_LINK,
  GOPHER_BUTTON,
  GOPHER_CHECKBOX,
  GOPHER_RADIO,
  GOPHER_OPTION,
  GOPHER_FIELD
};

struct GopherItem {
  char label[gopherLabelSize];
  uint8_t type;
  uint8_t target;
  uint8_t group;
  bool checked;
};

struct GopherPage {
  GopherItem items[maximumGopherItems];
  char targets[gopherTargetStorageSize];
  uint16_t targetOffsets[maximumGopherTargets];
  uint16_t targetBytes;
  uint8_t count;
  uint8_t targetCount;
  bool truncated;
};

GopherPage *gopherPageStorage = nullptr;
#define gopherPage (*gopherPageStorage)

class GopherPageSession {
public:
  GopherPageSession()
      : page(new (std::nothrow) GopherPage) {
    gopherPageStorage = page;
  }

  ~GopherPageSession() {
    gopherPageStorage = nullptr;
    delete page;
  }

  explicit operator bool() const { return page != nullptr; }

private:
  GopherPage *page;
};

extern const uint8_t rootCertificateBundle[]
    asm("_binary_x509_crt_bundle_start");

WiFiClient plainHttpClient;
WiFiClientSecure secureHttpClient;

struct CertificateBundleInitializer {
  CertificateBundleInitializer() {
    secureHttpClient.setCACertBundle(rootCertificateBundle);
    secureHttpClient.setHandshakeTimeout(8);
  }
};

CertificateBundleInitializer certificateBundleInitializer;

uint16_t backgroundColor() {
  return WatchyUi::Theme::background();
}

uint16_t foregroundColor() {
  return WatchyUi::Theme::foreground();
}

void beginScreen(const char *title) {
  WatchyUi::Screen::begin(title);
}

void finishScreen() {
  WatchyUi::Screen::present();
}

uint32_t waitForReleaseDuration(WatchyUi::Event event) {
  uint32_t startedAt = millis();
  WatchyUi::Input::waitForRelease(event);
  return millis() - startedAt;
}

String clipped(const String &value, size_t length) {
  if (value.length() <= length) {
    return value;
  }
  if (length <= 3) {
    return value.substring(0, length);
  }
  return value.substring(0, length - 3) + "...";
}

void drawEditor(const char *title, const String &value, char selected) {
  beginScreen(title);
  String visible = value;
  if (visible.length() > 90) {
    visible = "..." + visible.substring(visible.length() - 87);
  }
  WatchyUi::GrayPaint::fillRoundRect(
      {8, 30, 184, 60}, 4,
      WatchyUi::Theme::tone(WatchyUi::ToneRole::Surface));
  WatchyUi::Widget::paragraph(visible.c_str(), 12, 42, 29, 5, 10);
  AppVisual::drawStatusIcon({28, 111, 32, 32}, AppVisual::StatusIcon::INFO,
                            true);
  constexpr WatchyUi::Bounds selectedBounds{78, 104, 44, 42};
  Watchy::display.drawRect(selectedBounds.x, selectedBounds.y,
                           selectedBounds.width, selectedBounds.height,
                           foregroundColor());
  char selectedText[2] = {selected, '\0'};
  Watchy::display.setTextSize(3);
  WatchyUi::Canvas::centeredText(selectedBounds, selectedText,
                                foregroundColor());
  WatchyUi::Widget::footer("UP/DOWN CHAR  SELECT ADD/HOLD RUN  BACK DEL/CLEAR");
  finishScreen();
}

String decodeEntities(const String &input) {
  String output;
  output.reserve(input.length());
  for (size_t index = 0; index < input.length(); index++) {
    if (input[index] != '&') {
      output += input[index];
      continue;
    }
    int end = input.indexOf(';', index + 1);
    if (end < 0 || end - static_cast<int>(index) > 10) {
      output += input[index];
      continue;
    }
    String entity = input.substring(index + 1, end);
    if (entity == "amp") output += '&';
    else if (entity == "lt") output += '<';
    else if (entity == "gt") output += '>';
    else if (entity == "quot") output += '"';
    else if (entity == "apos" || entity == "#39") output += '\'';
    else if (entity == "nbsp") output += ' ';
    else if (entity.startsWith("#x") || entity.startsWith("#X")) {
      long value = strtol(entity.substring(2).c_str(), nullptr, 16);
      output += value >= 32 && value <= 126 ? static_cast<char>(value) : '?';
    } else if (entity.startsWith("#")) {
      long value = strtol(entity.substring(1).c_str(), nullptr, 10);
      output += value >= 32 && value <= 126 ? static_cast<char>(value) : '?';
    } else {
      output += '&';
      output += entity;
      output += ';';
    }
    index = end;
  }
  return output;
}

char asciiLower(char value) {
  return value >= 'A' && value <= 'Z' ? value + ('a' - 'A') : value;
}

int indexOfIgnoreCase(const String &text, const char *needle,
                      size_t from = 0) {
  size_t needleLength = strlen(needle);
  if (needleLength == 0) {
    return from <= text.length() ? static_cast<int>(from) : -1;
  }
  if (needleLength > text.length() || from > text.length() - needleLength) {
    return -1;
  }
  for (size_t start = from; start <= text.length() - needleLength; start++) {
    size_t offset = 0;
    while (offset < needleLength &&
           asciiLower(text[start + offset]) == asciiLower(needle[offset])) {
      offset++;
    }
    if (offset == needleLength) return static_cast<int>(start);
  }
  return -1;
}

bool isBlockTag(const String &tag) {
  return tag.startsWith("br") || tag.startsWith("/p") ||
         tag.startsWith("/div") || tag.startsWith("/li") ||
         tag.startsWith("/h") || tag.startsWith("/tr") ||
         tag.startsWith("hr");
}

String attributeValue(const String &tag, const char *attribute) {
  String lower = tag;
  lower.toLowerCase();
  String needle = String(attribute) + "=";
  int position = lower.indexOf(needle);
  if (position < 0) {
    return "";
  }
  position += needle.length();
  while (position < static_cast<int>(tag.length()) && tag[position] == ' ') {
    position++;
  }
  if (position >= static_cast<int>(tag.length())) {
    return "";
  }
  char quote = tag[position];
  if (quote == '\'' || quote == '"') {
    int end = tag.indexOf(quote, position + 1);
    return end < 0 ? "" : tag.substring(position + 1, end);
  }
  int end = position;
  while (end < static_cast<int>(tag.length()) && tag[end] != ' ' &&
         tag[end] != '>') {
    end++;
  }
  return tag.substring(position, end);
}

bool fetchResource(const String &url, String &data,
                   String &contentType, size_t maximumBytes,
                   bool &truncated) {
  data = "";
  contentType = "";
  truncated = false;
  if (url.length() == 0 || url.length() > maximumUrlLength) return false;
  HTTPClient http;
  plainHttpClient.stop();
  secureHttpClient.stop();
  auto finishRequest = [&]() {
    http.end();
    plainHttpClient.stop();
    secureHttpClient.stop();
  };

  const char *headerNames[] = {"Content-Type"};
  http.collectHeaders(headerNames, 1);
  http.setConnectTimeout(8000);
  http.setTimeout(8000);
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  http.setRedirectLimit(4);
  http.setUserAgent("Watchy-Networking/1.0");

  bool https = url.startsWith("https://");
  bool started = https
                     ? http.begin(secureHttpClient, url)
                     : http.begin(plainHttpClient, url);
  if (!started) {
    if (https) contentType = "error/https";
    finishRequest();
    return false;
  }
  http.addHeader("Accept-Encoding", "identity");
  http.addHeader(
      "Accept",
      "text/html,application/xhtml+xml,application/rss+xml,"
      "application/atom+xml,application/xml,text/xml,text/plain;q=0.9");

  int status = http.GET();
  if (status < 200 || status >= 300) {
    if (https && status < 0) contentType = "error/https";
    finishRequest();
    return false;
  }

  contentType = http.header("Content-Type");
  String lowerType = contentType;
  lowerType.toLowerCase();
  if (lowerType.indexOf("image/") >= 0) {
    finishRequest();
    return false;
  }
  size_t downloadLimit = std::min(maximumBytes, maximumTextBytes);
  size_t largestBlock = ESP.getMaxAllocHeap();
  if (largestBlock <= networkHeapReserve + minimumDownloadBytes) {
    finishRequest();
    return false;
  }
  downloadLimit = std::min(downloadLimit, largestBlock - networkHeapReserve);
  int contentLength = http.getSize();
  if (contentLength >= 0) {
    downloadLimit = std::min(downloadLimit,
                             static_cast<size_t>(contentLength));
  }
  CappedStream sink(data, downloadLimit);
  if (!sink.ready()) {
    finishRequest();
    return false;
  }
  int written = http.writeToStream(&sink);
  truncated = sink.overflowed;
  WatchyDiagnostics::recordHeapCheckpoint(
      WatchyDiagnostics::HeapCheckpoint::Downloaded);
  finishRequest();
  return !sink.failed() &&
         (written >= 0 || (data.length() > 0 && truncated));
}

void drawLinkPage(const char *title, const LinkList &links,
                  uint8_t selected) {
  beginScreen(title);
  uint8_t first = selected >= linkRowsPerPage
                      ? selected - linkRowsPerPage + 1
                      : 0;
  uint8_t last = std::min<uint8_t>(links.count, first + linkRowsPerPage);
  Watchy::display.setFont();
  Watchy::display.setTextSize(1);
  for (uint8_t index = first; index < last; index++) {
    int row = index - first;
    int y = 25 + row * 32;
    bool active = index == selected;
    if (active) {
      Watchy::display.fillRect(0, y - 2, DISPLAY_WIDTH, 30,
                               foregroundColor());
    }
    Watchy::display.setTextColor(active ? backgroundColor()
                                        : foregroundColor());
    Watchy::display.setCursor(3, y + 2);
    Watchy::display.print(clipped(links.entries[index].title, 31));
    Watchy::display.setCursor(3, y + 14);
    Watchy::display.print(clipped(links.entries[index].url, 31));
  }
  finishScreen();
}

void drawFeedPage(const char *title, const FeedList &items,
                  uint8_t selected) {
  beginScreen(title);
  uint8_t first = selected >= feedRowsPerPage
                      ? selected - feedRowsPerPage + 1
                      : 0;
  uint8_t last = std::min<uint8_t>(items.count,
                                   first + feedRowsPerPage);
  Watchy::display.setFont();
  Watchy::display.setTextSize(1);
  for (uint8_t index = first; index < last; index++) {
    int row = index - first;
    int y = 25 + row * 37;
    bool active = index == selected;
    if (active) {
      Watchy::display.fillRect(0, y - 2, DISPLAY_WIDTH, 36,
                               foregroundColor());
    }
    Watchy::display.setTextColor(active ? backgroundColor()
                                        : foregroundColor());
    const String &headline = items.entries[index].title;
    for (uint8_t line = 0; line < feedTitleLines; line++) {
      size_t offset = line * feedTitleColumns;
      if (offset >= headline.length()) break;
      Watchy::display.setCursor(3, y + 1 + line * 8);
      String fragment = headline.substring(
          offset, std::min<size_t>(headline.length(),
                                   offset + feedTitleColumns));
      if (line == feedTitleLines - 1 &&
          headline.length() > offset + feedTitleColumns) {
        fragment = clipped(headline.substring(offset), feedTitleColumns);
      }
      Watchy::display.print(fragment);
    }
  }
  Watchy::display.setTextColor(foregroundColor());
  WatchyUi::Widget::footer("UP/DOWN SELECT      BACK EXIT");
  finishScreen();
}

size_t skipDnsName(const uint8_t *packet, size_t length, size_t offset) {
  while (offset < length) {
    uint8_t labelLength = packet[offset++];
    if (labelLength == 0) {
      return offset;
    }
    if ((labelLength & 0xc0) == 0xc0) {
      return std::min(length, offset + 1);
    }
    offset += labelLength;
  }
  return length;
}

String readDnsName(const uint8_t *packet, size_t length, size_t offset,
                   uint8_t depth = 0) {
  if (depth > 8) {
    return "";
  }
  String name;
  while (offset < length) {
    uint8_t labelLength = packet[offset++];
    if (labelLength == 0) {
      break;
    }
    if ((labelLength & 0xc0) == 0xc0) {
      if (offset >= length) {
        break;
      }
      size_t pointer = ((labelLength & 0x3f) << 8) | packet[offset];
      String suffix = readDnsName(packet, length, pointer, depth + 1);
      if (name.length() > 0 && suffix.length() > 0) name += '.';
      name += suffix;
      break;
    }
    if (offset + labelLength > length) {
      break;
    }
    if (name.length() > 0) name += '.';
    for (uint8_t index = 0; index < labelLength; index++) {
      name += static_cast<char>(packet[offset++]);
    }
  }
  return name;
}

uint16_t readNetwork16(const uint8_t *data) {
  return static_cast<uint16_t>(data[0]) << 8 | data[1];
}

uint16_t packetChecksum(const uint8_t *data, size_t length) {
  uint32_t sum = 0;
  for (size_t index = 0; index + 1 < length; index += 2) {
    sum += static_cast<uint16_t>(data[index]) << 8 | data[index + 1];
  }
  if (length & 1) {
    sum += static_cast<uint16_t>(data[length - 1]) << 8;
  }
  while (sum >> 16) {
    sum = (sum & 0xffff) + (sum >> 16);
  }
  return static_cast<uint16_t>(~sum);
}

bool matchesEchoIdentifier(const uint8_t *packet, int length, int offset,
                           uint16_t identifier) {
  return offset + 8 <= length && readNetwork16(packet + offset + 4) == identifier;
}

} // namespace

void prepareButtons() {
  WatchyUi::Input::begin();
}

Button waitForButton() {
  switch (WatchyUi::Input::wait()) {
  case WatchyUi::Event::MENU: return BUTTON_MENU;
  case WatchyUi::Event::BACK: return BUTTON_BACK;
  case WatchyUi::Event::UP: return BUTTON_UP;
  case WatchyUi::Event::DOWN: return BUTTON_DOWN;
  default: return BUTTON_NONE;
  }
}

bool backPressed() {
  return WatchyUi::Input::poll() == WatchyUi::Event::BACK;
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderTextEditorPreview(const char *title, const String &value,
                             char selected) {
  drawEditor(title, value, selected);
}
#endif

bool editText(const char *title, const String &initialValue, String &value,
              size_t maximumLength) {
  prepareButtons();
  value = initialValue;
  if (value.length() > maximumLength) {
    value.remove(maximumLength);
  }
  value.reserve(maximumLength + 1);
  size_t selectedIndex = 1;
  drawEditor(title, value, editorCharacters[selectedIndex]);
  while (true) {
    WatchyUi::Event event = WatchyUi::Input::wait();
    if (event == WatchyUi::Event::UP) {
      WatchyUi::Input::waitForRelease(event);
      selectedIndex = (selectedIndex + 1) % (sizeof(editorCharacters) - 1);
      drawEditor(title, value, editorCharacters[selectedIndex]);
    } else if (event == WatchyUi::Event::DOWN) {
      WatchyUi::Input::waitForRelease(event);
      selectedIndex = selectedIndex == 0
                          ? sizeof(editorCharacters) - 2
                          : selectedIndex - 1;
      drawEditor(title, value, editorCharacters[selectedIndex]);
    } else if (event == WatchyUi::Event::MENU) {
      uint32_t duration = waitForReleaseDuration(event);
      if (duration >= longPressMs) {
        value.trim();
        return value.length() > 0;
      }
      if (value.length() < maximumLength) {
        value += editorCharacters[selectedIndex];
      }
      drawEditor(title, value, editorCharacters[selectedIndex]);
    } else if (event == WatchyUi::Event::BACK) {
      uint32_t duration = waitForReleaseDuration(event);
      if (duration >= longPressMs) {
        if (value.length() == 0) return false;
        value = "";
      } else if (value.length() == 0) {
        return false;
      } else {
        value.remove(value.length() - 1);
      }
      drawEditor(title, value, editorCharacters[selectedIndex]);
    }
  }
}

void showStatus(const char *title, const String &message) {
  bool failure = message.indexOf("failed") >= 0 ||
                 message.indexOf("unavailable") >= 0 ||
                 message.indexOf("No ") >= 0;
  beginScreen(title);
  AppVisual::drawStatusIcon({79, 42, 42, 42},
                            failure ? AppVisual::StatusIcon::WARNING
                                    : AppVisual::StatusIcon::RADIO,
                            true);
  WatchyUi::Canvas::centeredText({0, 94, 200, 17},
                                 failure ? "NETWORK STATUS" : "WORKING",
                                 2, foregroundColor());
  WatchyUi::Widget::paragraph(message.c_str(), 14, 126, 29, 4, 10);
  WatchyUi::Widget::footer(failure ? "BACK RETURN" : "BACK CANCEL");
  finishScreen();
}

bool ensureConnected(const char *title, Watchy *watchy) {
  if (WiFi.status() == WL_CONNECTED) {
    WatchyDiagnostics::recordHeapCheckpoint(
        WatchyDiagnostics::HeapCheckpoint::Connected);
    return true;
  }
  showStatus(title, "Connecting WiFi...");
  if (watchy != nullptr ? watchy->connectWiFi() : WatchySdk::connectWiFi()) {
    WatchyDiagnostics::recordHeapCheckpoint(
      WatchyDiagnostics::HeapCheckpoint::Connected);
    return true;
  }
  showStatus(title, "WiFi unavailable\n\nBACK to return");
  prepareButtons();
  while (waitForButton() != BUTTON_BACK) {}
  return false;
}

void disconnect() {
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  btStop();
  WatchyDiagnostics::endWifiSession();
  Watchy::setLowPowerCpuFrequency();
}

void addLine(TextDocument &document, const String &line) {
  if (document.count < maximumDocumentLines) {
    document.lines[document.count++] = line;
  }
}

void makeDocument(const String &text, TextDocument &document,
                  uint8_t columns) {
  document.count = 0;
  String line;
  String word;
  auto flushWord = [&]() {
    while (word.length() > 0) {
      size_t room = line.length() == 0 ? columns : columns - line.length() - 1;
      if (word.length() <= room) {
        if (line.length() > 0) line += ' ';
        line += word;
        word = "";
      } else if (line.length() > 0) {
        addLine(document, line);
        line = "";
      } else {
        addLine(document, word.substring(0, columns));
        word.remove(0, columns);
      }
      if (document.count >= maximumDocumentLines) break;
    }
  };

  for (size_t index = 0; index <= text.length() &&
                         document.count < maximumDocumentLines; index++) {
    char character = index < text.length() ? text[index] : '\n';
    if (character == '\r') continue;
    if (character == '\n') {
      flushWord();
      if (line.length() > 0) {
        addLine(document, line);
        line = "";
      } else if (document.count > 0 &&
                 document.lines[document.count - 1].length() > 0) {
        addLine(document, "");
      }
    } else if (isspace(static_cast<unsigned char>(character))) {
      flushWord();
    } else {
      word += character;
    }
  }
  if (document.count == 0) addLine(document, "(empty)");
}

void drawDocumentPage(const char *title, const TextDocument &document,
                      uint8_t page) {
  beginScreen(title);
  uint8_t pageCount = std::max<uint8_t>(
      1, (document.count + documentLinesPerPage - 1) / documentLinesPerPage);
  page = std::min<uint8_t>(page, pageCount - 1);
  uint8_t first = page * documentLinesPerPage;
  uint8_t last = std::min<uint8_t>(document.count,
                                   first + documentLinesPerPage);
  Watchy::display.setFont();
  Watchy::display.setTextSize(1);
  Watchy::display.setTextColor(foregroundColor());
  for (uint8_t index = first; index < last; index++) {
    Watchy::display.setCursor(0, 27 + (index - first) * 8);
    Watchy::display.print(document.lines[index]);
  }
  Watchy::display.setCursor(168, 194);
  Watchy::display.print(page + 1);
  Watchy::display.print('/');
  Watchy::display.print(pageCount);
  finishScreen();
}

void viewDocument(const char *title, const TextDocument &document) {
  prepareButtons();
  uint8_t page = 0;
  uint8_t pageCount = std::max<uint8_t>(
      1, (document.count + documentLinesPerPage - 1) / documentLinesPerPage);
  drawDocumentPage(title, document, page);
  while (true) {
    Button button = waitForButton();
    if (button == BUTTON_BACK) return;
    if (button == BUTTON_UP) {
      page = page == 0 ? pageCount - 1 : page - 1;
      drawDocumentPage(title, document, page);
    } else if (button == BUTTON_DOWN) {
      page = (page + 1) % pageCount;
      drawDocumentPage(title, document, page);
    }
  }
}

bool fetchText(const String &url, String &body, String &contentType,
               size_t maximumBytes) {
  bool truncated = false;
  return fetchResource(normalizeUrl(url), body, contentType, maximumBytes,
                       truncated);
}

String normalizeUrl(const String &url) {
  String normalized = url;
  normalized.trim();
  if (!normalized.startsWith("http://") &&
      !normalized.startsWith("https://")) {
    normalized = "https://" + normalized;
  }
  return normalized;
}

String resolveUrl(const String &baseUrl, const String &reference) {
  String ref = reference;
  ref.trim();
  if (ref.startsWith("http://") || ref.startsWith("https://")) return ref;
  int schemeEnd = baseUrl.indexOf("://");
  if (schemeEnd < 0) return normalizeUrl(ref);
  String scheme = baseUrl.substring(0, schemeEnd);
  int hostStart = schemeEnd + 3;
  int pathStart = baseUrl.indexOf('/', hostStart);
  String origin = pathStart < 0 ? baseUrl : baseUrl.substring(0, pathStart);
  if (ref.startsWith("//")) return scheme + ":" + ref;
  if (ref.startsWith("/")) return origin + ref;
  String directory = pathStart < 0 ? origin + "/" :
      baseUrl.substring(0, baseUrl.lastIndexOf('/') + 1);
  return directory + ref;
}

String urlEncode(const String &value) {
  const char hex[] = "0123456789ABCDEF";
  String encoded;
  for (size_t index = 0; index < value.length(); index++) {
    uint8_t character = value[index];
    if (isalnum(character) || character == '-' || character == '_' ||
        character == '.' || character == '~') {
      encoded += static_cast<char>(character);
    } else {
      encoded += '%';
      encoded += hex[character >> 4];
      encoded += hex[character & 0x0f];
    }
  }
  return encoded;
}

String urlDecode(const String &value) {
  String decoded;
  for (size_t index = 0; index < value.length(); index++) {
    if (value[index] == '%' && index + 2 < value.length()) {
      char digits[3] = {value[index + 1], value[index + 2], 0};
      decoded += static_cast<char>(strtol(digits, nullptr, 16));
      index += 2;
    } else if (value[index] == '+') {
      decoded += ' ';
    } else {
      decoded += value[index];
    }
  }
  return decoded;
}

String htmlToText(const String &html) {
  String output;
  output.reserve(std::min<size_t>(html.length(), 12000));
  for (size_t index = 0; index < html.length() && output.length() < 12000;
       index++) {
    if (html[index] == '<') {
      int end = html.indexOf('>', index + 1);
      if (end < 0) break;
      String tag = html.substring(index + 1, end);
      tag.trim();
      tag.toLowerCase();
      const char *closingTag = tag.startsWith("script") ? "</script" :
                               tag.startsWith("style") ? "</style" : nullptr;
      if (closingTag != nullptr) {
        int close = indexOfIgnoreCase(html, closingTag, end + 1);
        if (close < 0) break;
        int closeEnd = html.indexOf('>', close + strlen(closingTag));
        if (closeEnd < 0) break;
        index = closeEnd;
        continue;
      }
      if (isBlockTag(tag) && !output.endsWith("\n")) output += '\n';
      index = end;
    } else {
      output += html[index];
    }
  }
  output = decodeEntities(output);
  while (output.indexOf("\n\n\n") >= 0) output.replace("\n\n\n", "\n\n");
  return output;
}

void htmlToTextInPlace(String &html, size_t maximumCharacters) {
  const size_t inputLength = html.length();
  maximumCharacters = std::min(maximumCharacters, inputLength);
  size_t read = 0;
  size_t write = 0;
  bool pendingSpace = false;

  auto emit = [&](char character) {
    if (write < maximumCharacters) html.setCharAt(write++, character);
  };
  auto emitBreak = [&]() {
    while (write > 0 && html[write - 1] == ' ') write--;
    if (write > 0 && html[write - 1] != '\n') emit('\n');
    pendingSpace = false;
  };
  auto equalsIgnoreCase = [&](size_t start, size_t end,
                              const char *expected) {
    size_t length = strlen(expected);
    if (end - start != length) return false;
    for (size_t offset = 0; offset < length; offset++) {
      if (asciiLower(html[start + offset]) != asciiLower(expected[offset])) {
        return false;
      }
    }
    return true;
  };

  while (read < inputLength && write < maximumCharacters) {
    char character = html[read];
    if (character == '<') {
      int tagEndValue = html.indexOf('>', read + 1);
      if (tagEndValue < 0) break;
      size_t tagEnd = static_cast<size_t>(tagEndValue);
      size_t nameStart = read + 1;
      while (nameStart < tagEnd &&
             isspace(static_cast<unsigned char>(html[nameStart]))) {
        nameStart++;
      }
      bool closing = nameStart < tagEnd && html[nameStart] == '/';
      if (closing) nameStart++;
      size_t nameEnd = nameStart;
      while (nameEnd < tagEnd &&
             !isspace(static_cast<unsigned char>(html[nameEnd])) &&
             html[nameEnd] != '/' && html[nameEnd] != '>') {
        nameEnd++;
      }
      auto isTag = [&](const char *name) {
        return equalsIgnoreCase(nameStart, nameEnd, name);
      };

      if (!closing && (isTag("script") || isTag("style") ||
                       isTag("svg") || isTag("picture"))) {
        const char *closingTag = isTag("script") ? "</script" :
                                 isTag("style") ? "</style" :
                                 isTag("svg") ? "</svg" : "</picture";
        int close = indexOfIgnoreCase(html, closingTag, tagEnd + 1);
        if (close < 0) break;
        int closeEnd = html.indexOf('>', close + strlen(closingTag));
        if (closeEnd < 0) break;
        read = static_cast<size_t>(closeEnd) + 1;
        continue;
      }

      bool heading = nameEnd == nameStart + 2 &&
                     asciiLower(html[nameStart]) == 'h' &&
                     html[nameStart + 1] >= '1' &&
                     html[nameStart + 1] <= '6';
      bool block = isTag("br") || isTag("hr") || isTag("p") ||
                   isTag("div") || isTag("li") || isTag("tr") ||
                   isTag("article") || isTag("section") || heading;
      if (block) emitBreak();
      read = tagEnd + 1;
      continue;
    }

    if (character == '&') {
      int entityEndValue = html.indexOf(';', read + 1);
      if (entityEndValue > 0 && entityEndValue - static_cast<int>(read) <= 10) {
        size_t entityStart = read + 1;
        size_t entityEnd = static_cast<size_t>(entityEndValue);
        char decoded = 0;
        if (equalsIgnoreCase(entityStart, entityEnd, "amp")) decoded = '&';
        else if (equalsIgnoreCase(entityStart, entityEnd, "lt")) decoded = '<';
        else if (equalsIgnoreCase(entityStart, entityEnd, "gt")) decoded = '>';
        else if (equalsIgnoreCase(entityStart, entityEnd, "quot")) decoded = '"';
        else if (equalsIgnoreCase(entityStart, entityEnd, "apos") ||
                 equalsIgnoreCase(entityStart, entityEnd, "#39")) {
          decoded = '\'';
        } else if (equalsIgnoreCase(entityStart, entityEnd, "nbsp")) {
          decoded = ' ';
        } else if (entityStart < entityEnd && html[entityStart] == '#') {
          bool hexadecimal = entityStart + 1 < entityEnd &&
                             (html[entityStart + 1] == 'x' ||
                              html[entityStart + 1] == 'X');
          size_t digit = entityStart + (hexadecimal ? 2 : 1);
          uint32_t value = 0;
          bool valid = digit < entityEnd;
          while (valid && digit < entityEnd) {
            char current = html[digit++];
            uint8_t number;
            if (current >= '0' && current <= '9') number = current - '0';
            else if (hexadecimal && current >= 'a' && current <= 'f') {
              number = current - 'a' + 10;
            } else if (hexadecimal && current >= 'A' && current <= 'F') {
              number = current - 'A' + 10;
            } else {
              valid = false;
              break;
            }
            value = value * (hexadecimal ? 16 : 10) + number;
          }
          if (valid) decoded = value >= 32 && value <= 126
                                   ? static_cast<char>(value) : '?';
        }
        if (decoded != 0) {
          character = decoded;
          read = entityEnd + 1;
        } else {
          read++;
        }
      } else {
        read++;
      }
    } else {
      read++;
    }

    if (character == '\r') continue;
    if (isspace(static_cast<unsigned char>(character))) {
      pendingSpace = write > 0 && html[write - 1] != '\n';
      continue;
    }
    if (pendingSpace) emit(' ');
    emit(character);
    pendingSpace = false;
  }

  while (write > 0 &&
         isspace(static_cast<unsigned char>(html[write - 1]))) {
    write--;
  }
  html.remove(write);
}

String xmlElementValue(const String &xml, int blockStart, int blockEnd,
                       const char *tagName,
                       size_t maximumLength = SIZE_MAX) {
  String opening = String("<") + tagName;
  int start = indexOfIgnoreCase(xml, opening.c_str(), blockStart);
  if (start < 0 || start >= blockEnd) return "";
  int contentStart = xml.indexOf('>', start);
  if (contentStart < 0 || contentStart >= blockEnd) return "";
  String closing = String("</") + tagName + ">";
  int end = indexOfIgnoreCase(xml, closing.c_str(), contentStart + 1);
  if (end < 0 || end > blockEnd) return "";
  size_t valueStart = static_cast<size_t>(contentStart + 1);
  size_t valueEnd = std::min<size_t>(static_cast<size_t>(end),
                                     valueStart + maximumLength);
  return xml.substring(valueStart, valueEnd);
}

String feedText(const String &markup) {
  String value = markup;
  value.trim();
  if (value.startsWith("<![CDATA[")) {
    int end = value.lastIndexOf("]]>");
    if (end >= 9) value.remove(end);
    value.remove(0, 9);
  }
  value = decodeEntities(value);
  value = htmlToText(value);
  value.trim();
  return value;
}

String singleLineText(const String &text) {
  String output;
  output.reserve(text.length());
  bool pendingSpace = false;
  for (size_t index = 0; index < text.length(); index++) {
    char character = text[index];
    if (isspace(static_cast<unsigned char>(character))) {
      pendingSpace = output.length() > 0;
    } else {
      if (pendingSpace) output += ' ';
      output += character;
      pendingSpace = false;
    }
  }
  output.trim();
  return output;
}

void extractLinks(const String &html, const String &baseUrl, LinkList &links) {
  links.count = 0;
  int start = indexOfIgnoreCase(html, "<a");
  while (start >= 0 && links.count < maximumLinks) {
    int tagEnd = html.indexOf('>', start);
    int close = tagEnd < 0 ? -1 :
        indexOfIgnoreCase(html, "</a", tagEnd + 1);
    if (tagEnd < 0 || close < 0) break;
    String href = decodeEntities(
        attributeValue(html.substring(start, tagEnd + 1), "href"));
    String title = htmlToText(html.substring(tagEnd + 1, close));
    title.trim();
    if (href.startsWith("//duckduckgo.com/l/") ||
        href.startsWith("https://duckduckgo.com/l/")) {
      int parameter = href.indexOf("uddg=");
      if (parameter >= 0) {
        String target = href.substring(parameter + 5);
        int separator = target.indexOf('&');
        if (separator >= 0) target.remove(separator);
        href = urlDecode(target);
      }
    }
    if (title.length() > 0 && href.length() > 0 &&
        !href.startsWith("#") && !href.startsWith("javascript:") &&
        !href.startsWith("mailto:")) {
      links.entries[links.count].title = title;
      links.entries[links.count].url = resolveUrl(baseUrl, href);
      links.count++;
    }
    start = indexOfIgnoreCase(html, "<a", close + 3);
  }
}

void extractFeed(const String &xml, const String &baseUrl, FeedList &items) {
  items.count = 0;
  bool atom = indexOfIgnoreCase(xml, "<item") < 0;
  const char *opening = atom ? "<entry" : "<item";
  const char *closing = atom ? "</entry>" : "</item>";
  int start = indexOfIgnoreCase(xml, opening);
  while (start >= 0 && items.count < maximumLinks) {
    int blockStart = xml.indexOf('>', start);
    int blockEnd = blockStart < 0 ? -1 :
        indexOfIgnoreCase(xml, closing, blockStart + 1);
    if (blockStart < 0 || blockEnd < 0) break;
    String title = singleLineText(feedText(xmlElementValue(
        xml, blockStart, blockEnd, "title", maximumFeedTitleLength + 32)));
    if (title.length() == 0) title = "Untitled entry";
    if (title.length() > maximumFeedTitleLength) {
      title.remove(maximumFeedTitleLength);
    }

    String content = feedText(xmlElementValue(
        xml, blockStart, blockEnd, "content:encoded",
        maximumFeedContentLength + 32));
    if (content.length() == 0) {
      content = feedText(xmlElementValue(
          xml, blockStart, blockEnd, "description",
          maximumFeedContentLength + 32));
    }
    if (content.length() == 0) {
      content = feedText(xmlElementValue(
          xml, blockStart, blockEnd, "content",
          maximumFeedContentLength + 32));
    }
    if (content.length() == 0) {
      content = feedText(xmlElementValue(
          xml, blockStart, blockEnd, "summary",
          maximumFeedContentLength + 32));
    }
    if (content.length() > maximumFeedContentLength) {
      content.remove(maximumFeedContentLength);
    }

    String link;
    int linkStart = indexOfIgnoreCase(xml, "<link", blockStart + 1);
    if (linkStart >= blockEnd) linkStart = -1;
    int linkEnd = linkStart < 0 ? -1 : xml.indexOf('>', linkStart);
    if (linkStart >= 0 && linkEnd >= 0) {
      String linkTag = xml.substring(linkStart, linkEnd + 1);
      link = attributeValue(linkTag, "href");
      if (link.length() == 0 && !atom) {
        int closeLink = indexOfIgnoreCase(xml, "</link>", linkEnd + 1);
        if (closeLink >= 0 && closeLink < blockEnd) {
          link = xml.substring(linkEnd + 1, closeLink);
        }
      }
    }
    link = decodeEntities(link);
    link.trim();
    if (link.length() > maximumFeedUrlLength) link = "";
    if (content.length() > 0 || link.length() > 0) {
      items.entries[items.count].title = title;
      items.entries[items.count].content = content;
      items.entries[items.count].url = link.length() == 0
          ? String() : resolveUrl(baseUrl, link);
      items.count++;
    }
    start = indexOfIgnoreCase(xml, opening, blockEnd + strlen(closing));
  }
}

bool rangeEqualsIgnoreCase(const String &text, size_t start, size_t end,
                           const char *expected) {
  size_t length = strlen(expected);
  if (end - start != length) return false;
  for (size_t index = 0; index < length; index++) {
    if (asciiLower(text[start + index]) != asciiLower(expected[index])) {
      return false;
    }
  }
  return true;
}

bool textEqualsIgnoreCase(const char *text, const char *expected) {
  while (*text != 0 && *expected != 0) {
    if (asciiLower(*text++) != asciiLower(*expected++)) return false;
  }
  return *text == 0 && *expected == 0;
}

bool readTagName(const String &html, size_t tagStart, size_t tagEnd,
                 size_t &nameStart, size_t &nameEnd, bool &closing) {
  size_t cursor = tagStart + 1;
  while (cursor < tagEnd && isspace(static_cast<unsigned char>(html[cursor]))) {
    cursor++;
  }
  closing = cursor < tagEnd && html[cursor] == '/';
  if (closing) cursor++;
  while (cursor < tagEnd && isspace(static_cast<unsigned char>(html[cursor]))) {
    cursor++;
  }
  nameStart = cursor;
  while (cursor < tagEnd &&
         (isalnum(static_cast<unsigned char>(html[cursor])) ||
          html[cursor] == '-' || html[cursor] == '_')) {
    cursor++;
  }
  nameEnd = cursor;
  return nameEnd > nameStart;
}

bool findAttributeRange(const String &html, size_t tagStart, size_t tagEnd,
                        const char *attribute, size_t &valueStart,
                        size_t &valueEnd) {
  size_t cursor = tagStart + 1;
  if (cursor < tagEnd && html[cursor] == '/') cursor++;
  while (cursor < tagEnd &&
         !isspace(static_cast<unsigned char>(html[cursor])) &&
         html[cursor] != '>') {
    cursor++;
  }

  while (cursor < tagEnd) {
    while (cursor < tagEnd &&
           (isspace(static_cast<unsigned char>(html[cursor])) ||
            html[cursor] == '/')) {
      cursor++;
    }
    size_t nameStart = cursor;
    while (cursor < tagEnd &&
           !isspace(static_cast<unsigned char>(html[cursor])) &&
           html[cursor] != '=' && html[cursor] != '/' &&
           html[cursor] != '>') {
      cursor++;
    }
    size_t nameEnd = cursor;
    while (cursor < tagEnd && isspace(static_cast<unsigned char>(html[cursor]))) {
      cursor++;
    }
    bool matches = rangeEqualsIgnoreCase(html, nameStart, nameEnd, attribute);
    if (cursor >= tagEnd || html[cursor] != '=') {
      if (matches) {
        valueStart = valueEnd = cursor;
        return true;
      }
      if (nameEnd == nameStart) cursor++;
      continue;
    }

    cursor++;
    while (cursor < tagEnd && isspace(static_cast<unsigned char>(html[cursor]))) {
      cursor++;
    }
    char quote = cursor < tagEnd ? html[cursor] : 0;
    if (quote == '\'' || quote == '"') {
      cursor++;
      valueStart = cursor;
      while (cursor < tagEnd && html[cursor] != quote) cursor++;
      valueEnd = cursor;
      if (cursor < tagEnd) cursor++;
    } else {
      valueStart = cursor;
      while (cursor < tagEnd &&
             !isspace(static_cast<unsigned char>(html[cursor])) &&
             html[cursor] != '>') {
        cursor++;
      }
      valueEnd = cursor;
    }
    if (matches) return true;
  }
  return false;
}

bool decodeEntityCharacter(const String &text, size_t start, size_t end,
                           char &decoded, size_t &entityEnd) {
  size_t semicolon = start + 1;
  while (semicolon < end && semicolon - start <= 10 &&
         text[semicolon] != ';') {
    semicolon++;
  }
  if (semicolon >= end || text[semicolon] != ';') return false;
  size_t valueStart = start + 1;
  size_t valueEnd = semicolon;
  if (rangeEqualsIgnoreCase(text, valueStart, valueEnd, "amp")) decoded = '&';
  else if (rangeEqualsIgnoreCase(text, valueStart, valueEnd, "lt")) decoded = '<';
  else if (rangeEqualsIgnoreCase(text, valueStart, valueEnd, "gt")) decoded = '>';
  else if (rangeEqualsIgnoreCase(text, valueStart, valueEnd, "quot")) decoded = '"';
  else if (rangeEqualsIgnoreCase(text, valueStart, valueEnd, "apos") ||
           rangeEqualsIgnoreCase(text, valueStart, valueEnd, "#39")) {
    decoded = '\'';
  } else if (rangeEqualsIgnoreCase(text, valueStart, valueEnd, "nbsp")) {
    decoded = ' ';
  } else if (valueStart < valueEnd && text[valueStart] == '#') {
    int base = 10;
    size_t digit = valueStart + 1;
    if (digit < valueEnd && (text[digit] == 'x' || text[digit] == 'X')) {
      base = 16;
      digit++;
    }
    uint32_t value = 0;
    if (digit >= valueEnd) return false;
    for (; digit < valueEnd; digit++) {
      char character = text[digit];
      int number = character >= '0' && character <= '9'
                       ? character - '0'
                       : character >= 'a' && character <= 'f'
                             ? character - 'a' + 10
                             : character >= 'A' && character <= 'F'
                                   ? character - 'A' + 10
                                   : -1;
      if (number < 0 || number >= base) return false;
      value = value * base + number;
    }
    decoded = value >= 32 && value <= 126 ? static_cast<char>(value) : '?';
  } else {
    return false;
  }
  entityEnd = semicolon;
  return true;
}

void copyPlainTextRange(const String &html, size_t start, size_t end,
                        char *output, size_t outputSize) {
  size_t length = 0;
  bool pendingSpace = false;
  for (size_t cursor = start; cursor < end && length + 1 < outputSize;
       cursor++) {
    char character = html[cursor];
    if (character == '<') {
      int close = html.indexOf('>', cursor + 1);
      if (close < 0 || static_cast<size_t>(close) >= end) break;
      cursor = close;
      pendingSpace = length > 0;
      continue;
    }
    size_t entityEnd = cursor;
    if (character == '&' &&
        decodeEntityCharacter(html, cursor, end, character, entityEnd)) {
      cursor = entityEnd;
    }
    uint8_t byte = static_cast<uint8_t>(character);
    if (isspace(byte)) {
      pendingSpace = length > 0;
      continue;
    }
    if (byte >= 0x80) {
      if ((byte & 0xc0) == 0x80) continue;
      character = '?';
    }
    if (pendingSpace && length + 1 < outputSize) output[length++] = ' ';
    pendingSpace = false;
    if (length + 1 < outputSize) output[length++] = character;
  }
  output[length] = 0;
}

bool readAttributeString(const String &html, size_t tagStart, size_t tagEnd,
                         const char *attribute, String &value,
                         size_t maximumLength) {
  size_t valueStart = 0;
  size_t valueEnd = 0;
  if (!findAttributeRange(html, tagStart, tagEnd, attribute,
                          valueStart, valueEnd)) {
    value = "";
    return false;
  }
  value = "";
  value.reserve(std::min(maximumLength, valueEnd - valueStart) + 1);
  for (size_t cursor = valueStart; cursor < valueEnd; cursor++) {
    char character = html[cursor];
    size_t entityEnd = cursor;
    if (character == '&' && decodeEntityCharacter(
                                html, cursor, valueEnd, character, entityEnd)) {
      cursor = entityEnd;
    }
    if (value.length() >= maximumLength) return false;
    if (!value.concat(character)) {
      value = "";
      return false;
    }
  }
  return true;
}

void readAttributeLabel(const String &html, size_t tagStart, size_t tagEnd,
                        const char *attribute, char *label,
                        size_t labelSize = gopherLabelSize) {
  size_t valueStart = 0;
  size_t valueEnd = 0;
  if (findAttributeRange(html, tagStart, tagEnd, attribute,
                         valueStart, valueEnd)) {
    copyPlainTextRange(html, valueStart, valueEnd, label, labelSize);
  } else {
    label[0] = 0;
  }
}

void resetGopherPage(bool sourceTruncated) {
  gopherPage.count = 0;
  gopherPage.targetCount = 0;
  gopherPage.targetBytes = 0;
  gopherPage.truncated = sourceTruncated;
}

bool addGopherItem(GopherItemType type, const char *label,
                   uint8_t target = noGopherTarget, uint8_t group = 0,
                   bool checked = false) {
  if (gopherPage.count >= maximumGopherItems) {
    gopherPage.truncated = true;
    return false;
  }
  GopherItem &item = gopherPage.items[gopherPage.count++];
  strncpy(item.label, label, gopherLabelSize - 1);
  item.label[gopherLabelSize - 1] = 0;
  item.type = type;
  item.target = target;
  item.group = group;
  item.checked = checked;
  return true;
}

uint8_t storeGopherTarget(const String &url) {
  if (url.length() == 0 || url.length() > maximumUrlLength) {
    gopherPage.truncated = true;
    return noGopherTarget;
  }
  for (uint8_t index = 0; index < gopherPage.targetCount; index++) {
    if (url == gopherPage.targets + gopherPage.targetOffsets[index]) {
      return index;
    }
  }
  if (gopherPage.targetCount >= maximumGopherTargets ||
      gopherPage.targetBytes + url.length() + 1 > gopherTargetStorageSize) {
    gopherPage.truncated = true;
    return noGopherTarget;
  }
  uint8_t index = gopherPage.targetCount++;
  gopherPage.targetOffsets[index] = gopherPage.targetBytes;
  memcpy(gopherPage.targets + gopherPage.targetBytes, url.c_str(),
         url.length() + 1);
  gopherPage.targetBytes += url.length() + 1;
  return index;
}

uint8_t storeResolvedGopherTarget(const String &baseUrl,
                                  const String &reference) {
  if (reference.length() == 0 || reference.startsWith("#") ||
      reference.startsWith("javascript:") ||
      reference.startsWith("mailto:") || reference.startsWith("data:")) {
    return noGopherTarget;
  }
  String target = resolveUrl(baseUrl, reference);
  if (!target.startsWith("http://") && !target.startsWith("https://")) {
    return noGopherTarget;
  }
  return storeGopherTarget(target);
}

const char *gopherTarget(uint8_t target) {
  return target < gopherPage.targetCount
             ? gopherPage.targets + gopherPage.targetOffsets[target]
             : nullptr;
}

struct GopherTextBuilder {
  int16_t item = -1;
  char word[gopherLabelSize] = {};
  uint8_t wordLength = 0;
};

void appendGopherWord(GopherTextBuilder &builder) {
  if (builder.wordLength == 0) return;

  if (builder.item < 0) {
    if (!addGopherItem(GOPHER_TEXT, "")) {
      builder.wordLength = 0;
      return;
    }
    builder.item = gopherPage.count - 1;
  }

  GopherItem *item = &gopherPage.items[builder.item];
  size_t length = strlen(item->label);
  if (length > 0 &&
      length + 1 + builder.wordLength > gopherLabelSize - 1) {
    if (!addGopherItem(GOPHER_TEXT, "")) {
      builder.wordLength = 0;
      return;
    }
    builder.item = gopherPage.count - 1;
    item = &gopherPage.items[builder.item];
    length = 0;
  }

  if (length > 0) item->label[length++] = ' ';
  memcpy(item->label + length, builder.word, builder.wordLength);
  item->label[length + builder.wordLength] = 0;
  builder.wordLength = 0;
}

void finishGopherText(GopherTextBuilder &builder) {
  appendGopherWord(builder);
  builder.item = -1;
}

void appendGopherCharacter(GopherTextBuilder &builder, char character) {
  if (isspace(static_cast<unsigned char>(character))) {
    appendGopherWord(builder);
    return;
  }
  uint8_t byte = static_cast<uint8_t>(character);
  if (byte >= 0x80) {
    if ((byte & 0xc0) == 0x80) return;
    character = '?';
  }
  if (builder.wordLength >= gopherLabelSize - 1) {
    appendGopherWord(builder);
    builder.item = -1;
  }
  builder.word[builder.wordLength++] = character;
}

void appendGopherTextRange(const String &html, size_t start, size_t end,
                           GopherTextBuilder &builder) {
  for (size_t cursor = start; cursor < end; cursor++) {
    char character = html[cursor];
    size_t entityEnd = cursor;
    if (character == '&' &&
        decodeEntityCharacter(html, cursor, end, character, entityEnd)) {
      cursor = entityEnd;
    }
    appendGopherCharacter(builder, character);
  }
}

bool isBlockElement(const String &html, size_t nameStart, size_t nameEnd) {
  static const char *const names[] = {
      "address", "article", "aside", "blockquote", "br", "dd", "div",
      "dl", "dt", "fieldset", "figcaption", "figure", "footer", "form",
      "header", "hr", "li", "main", "nav", "ol", "p", "pre", "section",
      "table", "tbody", "td", "tfoot", "th", "thead", "tr", "ul"};
  for (const char *name : names) {
    if (rangeEqualsIgnoreCase(html, nameStart, nameEnd, name)) return true;
  }
  return false;
}

uint8_t attributeGroup(const String &html, size_t tagStart, size_t tagEnd,
                       uint8_t fallback) {
  size_t start = 0;
  size_t end = 0;
  if (!findAttributeRange(html, tagStart, tagEnd, "name", start, end) ||
      start == end) {
    return fallback;
  }
  uint8_t hash = 1;
  for (size_t cursor = start; cursor < end; cursor++) {
    hash = static_cast<uint8_t>(hash * 33 + asciiLower(html[cursor]));
  }
  return hash == 0 ? fallback : hash;
}

void buildGopherPage(const String &body, const String &baseUrl, bool html,
                     bool sourceTruncated) {
  resetGopherPage(sourceTruncated);
  GopherTextBuilder text;
  if (!html) {
    appendGopherTextRange(body, 0, body.length(), text);
    finishGopherText(text);
    if (gopherPage.count == 0) addGopherItem(GOPHER_TEXT, "(empty)");
    return;
  }

  uint8_t formTarget = noGopherTarget;
  uint8_t nextGroup = 1;
  size_t cursor = 0;
  while (cursor < body.length() && gopherPage.count < maximumGopherItems) {
    int nextTagValue = body.indexOf('<', cursor);
    size_t nextTag = nextTagValue < 0 ? body.length()
                                      : static_cast<size_t>(nextTagValue);
    appendGopherTextRange(body, cursor, nextTag, text);
    if (nextTag >= body.length()) break;

    if (body.startsWith("<!--", nextTag)) {
      int commentEnd = body.indexOf("-->", nextTag + 4);
      cursor = commentEnd < 0 ? body.length()
                              : static_cast<size_t>(commentEnd + 3);
      continue;
    }
    int tagEndValue = body.indexOf('>', nextTag + 1);
    if (tagEndValue < 0) break;
    size_t tagEnd = tagEndValue;
    size_t nameStart = 0;
    size_t nameEnd = 0;
    bool closing = false;
    if (!readTagName(body, nextTag, tagEnd, nameStart, nameEnd, closing)) {
      cursor = tagEnd + 1;
      continue;
    }

    if (closing) {
      if (rangeEqualsIgnoreCase(body, nameStart, nameEnd, "form")) {
        formTarget = noGopherTarget;
      }
      if (isBlockElement(body, nameStart, nameEnd)) finishGopherText(text);
      cursor = tagEnd + 1;
      continue;
    }

    if (rangeEqualsIgnoreCase(body, nameStart, nameEnd, "script") ||
        rangeEqualsIgnoreCase(body, nameStart, nameEnd, "style") ||
        rangeEqualsIgnoreCase(body, nameStart, nameEnd, "svg")) {
      const char *closingTag =
          rangeEqualsIgnoreCase(body, nameStart, nameEnd, "script")
              ? "</script"
              : rangeEqualsIgnoreCase(body, nameStart, nameEnd, "style")
                    ? "</style"
                    : "</svg";
      int close = indexOfIgnoreCase(body, closingTag, tagEnd + 1);
      int closeEnd = close < 0 ? -1 : body.indexOf('>', close + 2);
      cursor = closeEnd < 0 ? body.length()
                            : static_cast<size_t>(closeEnd + 1);
      finishGopherText(text);
      continue;
    }

    if (rangeEqualsIgnoreCase(body, nameStart, nameEnd, "form")) {
      String action;
      char method[8] = {};
      readAttributeLabel(body, nextTag, tagEnd, "method", method,
             sizeof(method));
      if (!textEqualsIgnoreCase(method, "post")) {
        if (readAttributeString(body, nextTag, tagEnd, "action", action,
                                maximumUrlLength) &&
            action.length() > 0) {
          formTarget = storeResolvedGopherTarget(baseUrl, action);
        } else {
          formTarget = storeGopherTarget(baseUrl);
        }
      } else {
        formTarget = noGopherTarget;
      }
      finishGopherText(text);
      cursor = tagEnd + 1;
      continue;
    }

    if (rangeEqualsIgnoreCase(body, nameStart, nameEnd, "a")) {
      int close = indexOfIgnoreCase(body, "</a", tagEnd + 1);
      size_t contentEnd = close < 0 ? tagEnd + 1
                                    : static_cast<size_t>(close);
      char label[gopherLabelSize] = {};
      copyPlainTextRange(body, tagEnd + 1, contentEnd, label,
                         sizeof(label));
      if (label[0] == 0) readAttributeLabel(body, nextTag, tagEnd, "title", label);
      if (label[0] == 0) strncpy(label, "link", sizeof(label) - 1);
      String href;
      uint8_t target = noGopherTarget;
      if (readAttributeString(body, nextTag, tagEnd, "href", href,
                              maximumUrlLength)) {
        target = storeResolvedGopherTarget(baseUrl, href);
      }
      finishGopherText(text);
      addGopherItem(target == noGopherTarget ? GOPHER_TEXT : GOPHER_LINK,
                    label, target);
      if (close >= 0) {
        int closeEnd = body.indexOf('>', close + 3);
        cursor = closeEnd < 0 ? contentEnd : static_cast<size_t>(closeEnd + 1);
      } else {
        cursor = tagEnd + 1;
      }
      continue;
    }

    bool heading = nameEnd - nameStart == 2 &&
                   asciiLower(body[nameStart]) == 'h' &&
                   body[nameStart + 1] >= '1' && body[nameStart + 1] <= '6';
    if (heading) {
      char closingTag[5] = {'<', '/', body[nameStart], body[nameStart + 1], 0};
      int close = indexOfIgnoreCase(body, closingTag, tagEnd + 1);
      size_t contentEnd = close < 0 ? tagEnd + 1
                                    : static_cast<size_t>(close);
      char label[gopherLabelSize] = {};
      copyPlainTextRange(body, tagEnd + 1, contentEnd, label,
                         sizeof(label));
      finishGopherText(text);
      if (label[0] != 0) addGopherItem(GOPHER_HEADING, label);
      if (close >= 0) {
        int closeEnd = body.indexOf('>', close + 3);
        cursor = closeEnd < 0 ? contentEnd : static_cast<size_t>(closeEnd + 1);
      } else {
        cursor = tagEnd + 1;
      }
      continue;
    }

    if (rangeEqualsIgnoreCase(body, nameStart, nameEnd, "button")) {
      int close = indexOfIgnoreCase(body, "</button", tagEnd + 1);
      size_t contentEnd = close < 0 ? tagEnd + 1
                                    : static_cast<size_t>(close);
      char label[gopherLabelSize] = {};
      copyPlainTextRange(body, tagEnd + 1, contentEnd, label,
                         sizeof(label));
      if (label[0] == 0) readAttributeLabel(body, nextTag, tagEnd, "value", label);
      if (label[0] == 0) strncpy(label, "button", sizeof(label) - 1);
      String action;
      uint8_t target = formTarget;
      if (readAttributeString(body, nextTag, tagEnd, "formaction", action,
                              maximumUrlLength) &&
          action.length() > 0) {
        target = storeResolvedGopherTarget(baseUrl, action);
      }
      finishGopherText(text);
      addGopherItem(GOPHER_BUTTON, label, target);
      if (close >= 0) {
        int closeEnd = body.indexOf('>', close + 3);
        cursor = closeEnd < 0 ? contentEnd : static_cast<size_t>(closeEnd + 1);
      } else {
        cursor = tagEnd + 1;
      }
      continue;
    }

    if (rangeEqualsIgnoreCase(body, nameStart, nameEnd, "input")) {
      char type[16] = {};
      char label[gopherLabelSize] = {};
      readAttributeLabel(body, nextTag, tagEnd, "type", type, sizeof(type));
      for (size_t index = 0; type[index] != 0; index++) {
        type[index] = asciiLower(type[index]);
      }
      if (type[0] == 0) strncpy(type, "text", sizeof(type) - 1);
      readAttributeLabel(body, nextTag, tagEnd, "aria-label", label);
      if (label[0] == 0) readAttributeLabel(body, nextTag, tagEnd, "name", label);
      if (label[0] == 0) readAttributeLabel(body, nextTag, tagEnd, "value", label);
      if (label[0] == 0) readAttributeLabel(body, nextTag, tagEnd, "placeholder", label);
      if (label[0] == 0) strncpy(label, type, sizeof(label) - 1);
      bool checked = findAttributeRange(body, nextTag, tagEnd, "checked",
                                        nameStart, nameEnd);
      finishGopherText(text);
      if (strcmp(type, "hidden") == 0 || strcmp(type, "image") == 0) {
      } else if (strcmp(type, "checkbox") == 0) {
        addGopherItem(GOPHER_CHECKBOX, label, noGopherTarget,
                      attributeGroup(body, nextTag, tagEnd, nextGroup++),
                      checked);
      } else if (strcmp(type, "radio") == 0) {
        addGopherItem(GOPHER_RADIO, label, noGopherTarget,
                      attributeGroup(body, nextTag, tagEnd, nextGroup++),
                      checked);
      } else if (strcmp(type, "submit") == 0 ||
                 strcmp(type, "button") == 0 ||
                 strcmp(type, "reset") == 0) {
        addGopherItem(GOPHER_BUTTON, label, formTarget);
      } else {
        addGopherItem(GOPHER_FIELD, label);
      }
      cursor = tagEnd + 1;
      continue;
    }

    if (rangeEqualsIgnoreCase(body, nameStart, nameEnd, "select")) {
      int close = indexOfIgnoreCase(body, "</select", tagEnd + 1);
      size_t selectEnd = close < 0 ? body.length()
                                   : static_cast<size_t>(close);
      char selectLabel[gopherLabelSize] = {};
      readAttributeLabel(body, nextTag, tagEnd, "aria-label", selectLabel);
      if (selectLabel[0] == 0) {
        readAttributeLabel(body, nextTag, tagEnd, "name", selectLabel);
      }
      finishGopherText(text);
      if (selectLabel[0] != 0) addGopherItem(GOPHER_FIELD, selectLabel);
      uint8_t group = nextGroup++;
      size_t optionCursor = tagEnd + 1;
      while (optionCursor < selectEnd &&
             gopherPage.count < maximumGopherItems) {
        int optionValue = indexOfIgnoreCase(body, "<option", optionCursor);
        if (optionValue < 0 || static_cast<size_t>(optionValue) >= selectEnd) break;
        size_t optionStart = optionValue;
        int optionTagEndValue = body.indexOf('>', optionStart + 1);
        if (optionTagEndValue < 0 ||
            static_cast<size_t>(optionTagEndValue) >= selectEnd) break;
        size_t optionTagEnd = optionTagEndValue;
        int optionClose = indexOfIgnoreCase(body, "</option", optionTagEnd + 1);
        int nextOption = indexOfIgnoreCase(body, "<option", optionTagEnd + 1);
        size_t optionContentEnd = optionClose >= 0 &&
                                          static_cast<size_t>(optionClose) < selectEnd
                                      ? static_cast<size_t>(optionClose)
                                      : nextOption >= 0 &&
                                                static_cast<size_t>(nextOption) < selectEnd
                                            ? static_cast<size_t>(nextOption)
                                            : selectEnd;
        char label[gopherLabelSize] = {};
        copyPlainTextRange(body, optionTagEnd + 1, optionContentEnd, label,
                           sizeof(label));
        if (label[0] == 0) {
          readAttributeLabel(body, optionStart, optionTagEnd, "value", label);
        }
        bool selected = findAttributeRange(body, optionStart, optionTagEnd,
                                           "selected", nameStart, nameEnd);
        if (label[0] != 0) {
          addGopherItem(GOPHER_OPTION, label, noGopherTarget, group, selected);
        }
        if (optionClose >= 0 &&
            static_cast<size_t>(optionClose) < selectEnd) {
          int optionCloseEnd = body.indexOf('>', optionClose + 3);
          optionCursor = optionCloseEnd < 0
                             ? optionContentEnd
                             : static_cast<size_t>(optionCloseEnd + 1);
        } else {
          optionCursor = optionContentEnd;
        }
      }
      if (close >= 0) {
        int closeEnd = body.indexOf('>', close + 3);
        cursor = closeEnd < 0 ? selectEnd : static_cast<size_t>(closeEnd + 1);
      } else {
        cursor = selectEnd;
      }
      continue;
    }

    if (rangeEqualsIgnoreCase(body, nameStart, nameEnd, "img") ||
        rangeEqualsIgnoreCase(body, nameStart, nameEnd, "picture") ||
        rangeEqualsIgnoreCase(body, nameStart, nameEnd, "video") ||
        rangeEqualsIgnoreCase(body, nameStart, nameEnd, "audio") ||
        rangeEqualsIgnoreCase(body, nameStart, nameEnd, "canvas")) {
      cursor = tagEnd + 1;
      continue;
    }

    if (isBlockElement(body, nameStart, nameEnd)) finishGopherText(text);
    cursor = tagEnd + 1;
  }
  finishGopherText(text);
  if (cursor < body.length()) gopherPage.truncated = true;
  if (gopherPage.count == 0) addGopherItem(GOPHER_TEXT, "(empty)");
}

const char *gopherPrefix(const GopherItem &item) {
  switch (item.type) {
  case GOPHER_HEADING: return "# ";
  case GOPHER_LINK: return "> ";
  case GOPHER_BUTTON: return "[>] ";
  case GOPHER_CHECKBOX: return item.checked ? "[x] " : "[ ] ";
  case GOPHER_RADIO: return item.checked ? "(*) " : "( ) ";
  case GOPHER_OPTION: return item.checked ? "(*) " : "( ) ";
  case GOPHER_FIELD: return ": ";
  default: return "  ";
  }
}

void drawGopherItemText(const GopherItem &item, int16_t rowTop) {
  const char *prefix = gopherPrefix(item);
  size_t prefixLength = strlen(prefix);
  size_t labelLength = strlen(item.label);
  size_t firstLineCapacity = gopherTextColumns - prefixLength;
  size_t firstLineLength = std::min(labelLength, firstLineCapacity);
  if (labelLength > firstLineCapacity) {
    size_t wordBoundary = firstLineCapacity;
    while (wordBoundary > 0 && item.label[wordBoundary] != ' ') {
      wordBoundary--;
    }
    if (wordBoundary > 0) firstLineLength = wordBoundary;
  }

  Watchy::display.setCursor(2, rowTop + 1);
  Watchy::display.print(prefix);
  for (size_t index = 0; index < firstLineLength; index++) {
    Watchy::display.write(item.label[index]);
  }

  if (firstLineLength < labelLength) {
    size_t secondLineStart = firstLineLength;
    while (item.label[secondLineStart] == ' ') secondLineStart++;
    Watchy::display.setCursor(2, rowTop + 9);
    Watchy::display.print(item.label + secondLineStart);
  }
}

void drawGopherScrollBar(uint8_t first) {
  if (gopherPage.count <= gopherRowsPerView) return;

  constexpr int16_t scrollBarWidth = 4;
  constexpr int16_t minimumThumbHeight = 12;
  int16_t contentHeight = DISPLAY_HEIGHT - gopherContentTop;
  int16_t thumbHeight = std::max<int16_t>(
      minimumThumbHeight,
      contentHeight * gopherRowsPerView / gopherPage.count);
  uint8_t maximumFirst = gopherPage.count - gopherRowsPerView;
  int16_t thumbTop = gopherContentTop +
      (contentHeight - thumbHeight) * first / maximumFirst;

  Watchy::display.fillRect(DISPLAY_WIDTH - scrollBarWidth, gopherContentTop,
                           scrollBarWidth, contentHeight, backgroundColor());
  Watchy::display.drawFastVLine(DISPLAY_WIDTH - 2, gopherContentTop,
                                contentHeight, foregroundColor());
  Watchy::display.fillRect(DISPLAY_WIDTH - scrollBarWidth, thumbTop,
                           scrollBarWidth, thumbHeight, foregroundColor());
}

void drawGopherPage(const String &url, uint8_t selected) {
  beginScreen("GOPHER");
  Watchy::display.setFont();
  Watchy::display.setTextSize(1);
  if (gopherPage.count == 0) {
    Watchy::display.setCursor(0, 28);
    Watchy::display.print("(empty)");
    finishScreen();
    return;
  }
  selected = std::min<uint8_t>(selected, gopherPage.count - 1);

  char status[18] = {};
  snprintf(status, sizeof(status), "%s%u/%u%s",
           url.startsWith("https://") ? "TLS " : "",
           static_cast<unsigned>(selected + 1),
           static_cast<unsigned>(gopherPage.count),
           gopherPage.truncated ? "+" : "");
  int16_t statusX = DISPLAY_WIDTH - strlen(status) * 6;
  Watchy::display.setCursor(std::max<int16_t>(0, statusX), 4);
  Watchy::display.print(status);
  Watchy::display.drawFastHLine(0, gopherContentTop - 1, DISPLAY_WIDTH,
                                foregroundColor());

  uint8_t first = selected >= gopherRowsPerView
                      ? selected - gopherRowsPerView + 1
                      : 0;
  uint8_t last = std::min<uint8_t>(gopherPage.count,
                                   first + gopherRowsPerView);
  constexpr int16_t contentHeight = DISPLAY_HEIGHT - gopherContentTop;
  Watchy::display.setTextSize(1);
  for (uint8_t index = first; index < last; index++) {
    uint8_t row = index - first;
    int16_t rowTop = gopherContentTop +
                     row * contentHeight / gopherRowsPerView;
    int16_t rowBottom = gopherContentTop +
                        (row + 1) * contentHeight / gopherRowsPerView;
    bool active = index == selected;
    if (active) {
      Watchy::display.fillRect(0, rowTop, DISPLAY_WIDTH,
                               rowBottom - rowTop,
                               foregroundColor());
    }
    Watchy::display.setTextColor(active ? backgroundColor()
                                        : foregroundColor());
    drawGopherItemText(gopherPage.items[index], rowTop);
  }
  Watchy::display.setTextColor(foregroundColor());
  Watchy::display.setTextSize(1);
  drawGopherScrollBar(first);
  finishScreen();
}

void toggleGopherItem(uint8_t selected) {
  GopherItem &item = gopherPage.items[selected];
  if (item.type == GOPHER_CHECKBOX) {
    item.checked = !item.checked;
    return;
  }
  if (item.type != GOPHER_RADIO && item.type != GOPHER_OPTION) return;
  for (uint8_t index = 0; index < gopherPage.count; index++) {
    GopherItem &candidate = gopherPage.items[index];
    if (candidate.group == item.group && candidate.type == item.type) {
      candidate.checked = index == selected;
    }
  }
}

int chooseLink(const char *title, const LinkList &links,
               uint8_t initialSelection) {
  if (links.count == 0) return -1;
  prepareButtons();
  uint8_t selected = std::min<uint8_t>(initialSelection, links.count - 1);
  drawLinkPage(title, links, selected);
  while (true) {
    Button button = waitForButton();
    if (button == BUTTON_BACK) return -1;
    if (button == BUTTON_MENU) return selected;
    if (button == BUTTON_UP) {
      selected = selected == 0 ? links.count - 1 : selected - 1;
      drawLinkPage(title, links, selected);
    } else if (button == BUTTON_DOWN) {
      selected = (selected + 1) % links.count;
      drawLinkPage(title, links, selected);
    }
  }
}

void drawFeedHeadlines(const char *title, const FeedList &items,
                       uint8_t selectedIndex) {
  if (items.count == 0) return;
  drawFeedPage(title, items,
               std::min<uint8_t>(selectedIndex, items.count - 1));
}

int chooseFeedItem(const char *title, const FeedList &items,
                   uint8_t initialSelection) {
  if (items.count == 0) return -1;
  prepareButtons();
  uint8_t selected = std::min<uint8_t>(initialSelection, items.count - 1);
  drawFeedHeadlines(title, items, selected);
  while (true) {
    Button button = waitForButton();
    if (button == BUTTON_BACK) return -1;
    if (button == BUTTON_MENU) return selected;
    if (button == BUTTON_UP) {
      selected = selected == 0 ? items.count - 1 : selected - 1;
      drawFeedHeadlines(title, items, selected);
    } else if (button == BUTTON_DOWN) {
      selected = (selected + 1) % items.count;
      drawFeedHeadlines(title, items, selected);
    }
  }
}

void browse(const String &initialUrl, Watchy *watchy) {
  GopherPageSession pageSession;
  if (!pageSession) {
    showStatus("BROWSER", "Not enough memory\n\nBACK to return");
    prepareButtons();
    while (waitForButton() != BUTTON_BACK) {}
    return;
  }
  String currentUrl = normalizeUrl(initialUrl);
  String history[4];
  uint8_t historyCount = 0;
  while (true) {
    if (!ensureConnected("BROWSER", watchy)) return;
    showStatus("BROWSER", "Loading\n" + clipped(currentUrl, 60));
    String resource;
    String contentType;
    bool truncated = false;
    bool fetched = fetchResource(currentUrl, resource, contentType,
                                 maximumTextBytes, truncated);
    disconnect();
    if (!fetched) {
      String lowerType = contentType;
      lowerType.toLowerCase();
        showStatus(
          "BROWSER",
          lowerType == "error/https"
            ? "HTTPS/TLS failed\nCheck clock/certificate\n\nBACK to return"
            : lowerType.indexOf("image/") >= 0
              ? "Images disabled\n\nBACK to return"
              : "Request failed\n\nBACK to return");
      if (waitForButton() == BUTTON_BACK) {
        if (historyCount == 0) return;
        currentUrl = history[--historyCount];
      }
      continue;
    }

    String lowerType = contentType;
    lowerType.toLowerCase();
    bool html = lowerType.indexOf("html") >= 0 || resource.indexOf('<') >= 0;
    buildGopherPage(resource, currentUrl, html, truncated);
    WatchyDiagnostics::recordHeapCheckpoint(
      WatchyDiagnostics::HeapCheckpoint::Parsed);
    resource = String();
    contentType = String();
    lowerType = String();

    uint8_t selected = 0;
    bool navigate = false;

    prepareButtons();
    while (!navigate) {
      drawGopherPage(currentUrl, selected);

      Button button = waitForButton();
      if (button == BUTTON_BACK) {
        if (historyCount == 0) return;
        currentUrl = history[--historyCount];
        navigate = true;
      } else if (button == BUTTON_UP) {
        if (gopherPage.count > 0) {
          selected = selected == 0 ? gopherPage.count - 1 : selected - 1;
        }
      } else if (button == BUTTON_DOWN) {
        if (gopherPage.count > 0) {
          selected = (selected + 1) % gopherPage.count;
        }
      } else if (button == BUTTON_MENU) {
        if (gopherPage.count == 0) continue;
        GopherItem &item = gopherPage.items[selected];
        const char *target = gopherTarget(item.target);
        if (target != nullptr) {
          String nextUrl = target;
          if (nextUrl.length() == 0 || nextUrl.length() > maximumUrlLength) {
            continue;
          }
          if (historyCount < 4) history[historyCount++] = currentUrl;
          currentUrl = std::move(nextUrl);
          navigate = true;
        } else {
          toggleGopherItem(selected);
        }
      }
    }
  }
}

bool resolveIPv4(const String &host, IPAddress &address) {
  String value = host;
  value.trim();
  if (address.fromString(value)) return true;
  return WiFi.hostByName(value.c_str(), address) == 1;
}

bool dnsQuery(const String &name, uint16_t queryType,
              TextDocument &document) {
  document.count = 0;
  uint8_t packet[512] = {};
  uint16_t identifier = static_cast<uint16_t>(esp_random());
  packet[0] = identifier >> 8;
  packet[1] = identifier & 0xff;
  packet[2] = 0x01;
  packet[5] = 0x01;
  size_t length = 12;

  int labelStart = 0;
  for (int index = 0; index <= static_cast<int>(name.length()); index++) {
    if (index == static_cast<int>(name.length()) || name[index] == '.') {
      int labelLength = index - labelStart;
      if (labelLength <= 0 || labelLength > 63 ||
          length + labelLength + 5 >= sizeof(packet)) return false;
      packet[length++] = labelLength;
      for (int character = labelStart; character < index; character++) {
        packet[length++] = name[character];
      }
      labelStart = index + 1;
    }
  }
  packet[length++] = 0;
  packet[length++] = queryType >> 8;
  packet[length++] = queryType & 0xff;
  packet[length++] = 0;
  packet[length++] = 1;

  IPAddress server = WiFi.dnsIP(0);
  if (server == IPAddress(0, 0, 0, 0)) server = IPAddress(1, 1, 1, 1);
  int socketHandle = lwip_socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (socketHandle < 0) return false;
  timeval timeout = {2, 500000};
  if (lwip_setsockopt(socketHandle, SOL_SOCKET, SO_RCVTIMEO, &timeout,
                      sizeof(timeout)) < 0) {
    lwip_close(socketHandle);
    return false;
  }

  sockaddr_in destination{};
  destination.sin_family = AF_INET;
  destination.sin_port = htons(53);
  destination.sin_addr.s_addr = static_cast<uint32_t>(server);
  if (lwip_sendto(socketHandle, packet, length, 0,
                  reinterpret_cast<sockaddr *>(&destination),
                  sizeof(destination)) != static_cast<int>(length)) {
    lwip_close(socketHandle);
    return false;
  }

  sockaddr_in sender{};
  socklen_t senderLength = sizeof(sender);
  int responseLength = lwip_recvfrom(
      socketHandle, packet, sizeof(packet), 0,
      reinterpret_cast<sockaddr *>(&sender), &senderLength);
  lwip_close(socketHandle);
  if (responseLength < 12 || readNetwork16(packet) != identifier) {
    return false;
  }

  uint16_t questionCount = readNetwork16(packet + 4);
  uint16_t answerCount = readNetwork16(packet + 6);
  size_t offset = 12;
  for (uint16_t question = 0; question < questionCount; question++) {
    offset = skipDnsName(packet, responseLength, offset);
    offset += 4;
  }
  for (uint16_t answer = 0; answer < answerCount && offset + 10 <=
       static_cast<size_t>(responseLength); answer++) {
    offset = skipDnsName(packet, responseLength, offset);
    if (offset + 10 > static_cast<size_t>(responseLength)) break;
    uint16_t type = readNetwork16(packet + offset);
    uint16_t dataLength = readNetwork16(packet + offset + 8);
    offset += 10;
    if (offset + dataLength > static_cast<size_t>(responseLength)) break;
    if (type == 1 && dataLength == 4) {
      IPAddress result(packet[offset], packet[offset + 1], packet[offset + 2],
                       packet[offset + 3]);
      addLine(document, "A     " + result.toString());
    } else if (type == 5) {
      addLine(document, "CNAME " +
                           readDnsName(packet, responseLength, offset));
    } else if (type == 12) {
      addLine(document, "PTR   " +
                           readDnsName(packet, responseLength, offset));
    }
    offset += dataLength;
  }
  if (document.count == 0) addLine(document, "No matching records");
  return true;
}

EchoResult sendEcho(const IPAddress &target, uint8_t ttl, uint16_t sequence,
                    uint32_t timeoutMs) {
  EchoResult result;
  int socketHandle = lwip_socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
  if (socketHandle < 0) return result;

  int ttlValue = ttl;
  lwip_setsockopt(socketHandle, IPPROTO_IP, IP_TTL, &ttlValue,
                  sizeof(ttlValue));
  timeval timeout = {static_cast<long>(timeoutMs / 1000),
                     static_cast<long>((timeoutMs % 1000) * 1000)};
  lwip_setsockopt(socketHandle, SOL_SOCKET, SO_RCVTIMEO, &timeout,
                  sizeof(timeout));

  constexpr size_t packetSize = 32;
  uint8_t packet[packetSize] = {};
  uint16_t identifier = static_cast<uint16_t>(esp_random());
  packet[0] = 8;
  packet[4] = identifier >> 8;
  packet[5] = identifier & 0xff;
  packet[6] = sequence >> 8;
  packet[7] = sequence & 0xff;
  for (size_t index = 8; index < packetSize; index++) packet[index] = index;
  uint16_t checksum = packetChecksum(packet, packetSize);
  packet[2] = checksum >> 8;
  packet[3] = checksum & 0xff;

  sockaddr_in destination = {};
  destination.sin_family = AF_INET;
  destination.sin_addr.s_addr = inet_addr(target.toString().c_str());
  uint32_t startedAt = millis();
  if (lwip_sendto(socketHandle, packet, packetSize, 0,
                  reinterpret_cast<sockaddr *>(&destination),
                  sizeof(destination)) < 0) {
    lwip_close(socketHandle);
    return result;
  }

  uint8_t response[160];
  sockaddr_in sender = {};
  socklen_t senderLength = sizeof(sender);
  while (millis() - startedAt <= timeoutMs) {
    int received = lwip_recvfrom(socketHandle, response, sizeof(response), 0,
                                 reinterpret_cast<sockaddr *>(&sender),
                                 &senderLength);
    if (received < 0) break;
    int ipHeaderLength = (response[0] & 0x0f) * 4;
    if (ipHeaderLength < 20 || ipHeaderLength + 8 > received) continue;
    uint8_t type = response[ipHeaderLength];
    bool matches = type == 0 && matchesEchoIdentifier(
        response, received, ipHeaderLength, identifier);
    if (type == 11 || type == 3) {
      int innerIp = ipHeaderLength + 8;
      if (innerIp + 20 <= received) {
        int innerHeaderLength = (response[innerIp] & 0x0f) * 4;
        matches = matchesEchoIdentifier(response, received,
                                        innerIp + innerHeaderLength,
                                        identifier);
      }
    }
    if (!matches) continue;
    result.responder = IPAddress(
        reinterpret_cast<uint8_t *>(&sender.sin_addr.s_addr));
    result.elapsedMs = millis() - startedAt;
    result.status = type == 0 ? ECHO_REPLY
                    : type == 11 ? ECHO_TTL_EXPIRED
                                 : ECHO_UNREACHABLE;
    lwip_close(socketHandle);
    return result;
  }

  result.status = ECHO_TIMEOUT;
  result.elapsedMs = millis() - startedAt;
  lwip_close(socketHandle);
  return result;
}

} // namespace NetworkApps
