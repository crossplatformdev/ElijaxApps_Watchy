#ifndef NETWORK_APP_COMMON_H
#define NETWORK_APP_COMMON_H

#include <Watchy.h>

namespace NetworkApps {

constexpr uint8_t maximumLinks = 12;
constexpr uint8_t maximumDocumentLines = 96;
constexpr size_t maximumUrlLength = 2000;

enum Button : uint8_t {
  BUTTON_NONE,
  BUTTON_MENU,
  BUTTON_BACK,
  BUTTON_UP,
  BUTTON_DOWN
};

struct Link {
  String title;
  String url;
};

struct LinkList {
  Link entries[maximumLinks];
  uint8_t count = 0;
};

struct TextDocument {
  String lines[maximumDocumentLines];
  uint8_t count = 0;
};

enum EchoStatus : uint8_t {
  ECHO_REPLY,
  ECHO_TTL_EXPIRED,
  ECHO_UNREACHABLE,
  ECHO_TIMEOUT,
  ECHO_ERROR
};

struct EchoResult {
  EchoStatus status = ECHO_ERROR;
  IPAddress responder;
  uint32_t elapsedMs = 0;
};

void prepareButtons();
Button waitForButton();
bool backPressed();
bool editText(const char *title, const String &initialValue, String &value,
              size_t maximumLength = 96);

void showStatus(const char *title, const String &message);
bool ensureConnected(Watchy &watchy, const char *title);
void disconnect();

void addLine(TextDocument &document, const String &line);
void makeDocument(const String &text, TextDocument &document,
                  uint8_t columns = 31);
void drawDocumentPage(const char *title, const TextDocument &document,
                      uint8_t page);
void viewDocument(const char *title, const TextDocument &document);

bool fetchText(const String &url, String &body, String &contentType,
               size_t maximumBytes = 12288);
String normalizeUrl(const String &url);
String resolveUrl(const String &baseUrl, const String &reference);
String urlEncode(const String &value);
String urlDecode(const String &value);
String htmlToText(const String &html);
void extractLinks(const String &html, const String &baseUrl, LinkList &links);
void extractFeed(const String &xml, const String &baseUrl, LinkList &items);
int chooseLink(const char *title, const LinkList &links,
               uint8_t initialSelection = 0);
void browse(Watchy &watchy, const String &initialUrl);

bool resolveIPv4(const String &host, IPAddress &address);
bool dnsQuery(const String &name, uint16_t queryType,
              TextDocument &document);
EchoResult sendEcho(const IPAddress &target, uint8_t ttl, uint16_t sequence,
                    uint32_t timeoutMs = 1000);

} // namespace NetworkApps

#endif