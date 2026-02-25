#ifndef NETWORK_APP_COMMON_H
#define NETWORK_APP_COMMON_H

#include <Watchy.h>
#include "AppDefaults.h"

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

struct FeedItem {
  String title;
  String content;
  String url;
};

struct FeedList {
  FeedItem entries[maximumLinks];
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
#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderTextEditorPreview(const char *title, const String &value,
                             char selected = 'A');
#endif

void showStatus(const char *title, const String &message);
bool ensureConnected(const char *title, Watchy *watchy = nullptr);
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
void htmlToTextInPlace(String &html, size_t maximumCharacters = 6144);
void extractLinks(const String &html, const String &baseUrl, LinkList &links);
void extractFeed(const String &xml, const String &baseUrl, FeedList &items);
void drawFeedHeadlines(const char *title, const FeedList &items,
                       uint8_t selectedIndex = 0);
int chooseLink(const char *title, const LinkList &links,
               uint8_t initialSelection = 0);
int chooseFeedItem(const char *title, const FeedList &items,
                   uint8_t initialSelection = 0);
void browse(const String &initialUrl, Watchy *watchy = nullptr);

bool resolveIPv4(const String &host, IPAddress &address);
bool dnsQuery(const String &name, uint16_t queryType,
              TextDocument &document);
EchoResult sendEcho(const IPAddress &target, uint8_t ttl, uint16_t sequence,
                    uint32_t timeoutMs = 1000);

} // namespace NetworkApps

#endif