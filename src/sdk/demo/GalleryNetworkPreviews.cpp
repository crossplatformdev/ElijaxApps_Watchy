#include "GalleryRenderers.h"

#include "WatchyUi.h"
#include "Watchy.h"

#ifdef WATCHY_DETERMINISTIC_GALLERY

#include "AppDisplay.h"
#include "NetworkAppCommon.h"


namespace WatchyDemo {
namespace {

const char *const titles[] = {
    "BROWSER", "RSS FEED", "PING", "TRACEROUTE", "PORT SCANNER",
    "DNS QUERY", "REVERSE DNS", "DUCKDUCKGO", "WIFI SURVEY"};

void drawNetworkLoading(const char *title, const char *detail) {
  WatchyUi::Screen::begin(title);
  AppVisual::drawStatusIcon({79, 43, 42, 42}, AppVisual::StatusIcon::RADIO,
                            true);
  WatchyUi::Canvas::centeredText({0, 96, 200, 18}, "WORKING", 2,
                                 WatchyUi::Theme::foreground());
  WatchyUi::Canvas::centeredText({12, 124, 176, 22}, detail, 1,
                                 WatchyUi::Theme::foreground());
  WatchyUi::Widget::footer("BACK CANCEL");
  WatchyUi::Screen::present();
}

void drawNetworkTerminal(const char *title, const char *label,
                         const char *detail, bool empty = false) {
  WatchyUi::Screen::begin(title);
  if (empty) {
    AppVisual::drawEmptyState({8, 39, 184, 118}, label, detail);
  } else {
    AppVisual::drawWarningState({8, 39, 184, 118}, label, detail);
  }
  WatchyUi::Widget::footer("BACK EXIT");
  WatchyUi::Screen::present();
}

void drawBrowserResult() {
  WatchyUi::Screen::begin("BROWSER");
  AppVisual::drawStatusIcon({79, 36, 42, 42}, AppVisual::StatusIcon::INFO,
                            true);
  WatchyUi::Canvas::centeredText({0, 87, 200, 17}, "WATCHY DEMO", 2,
                                 WatchyUi::Theme::foreground());
  AppVisual::drawDataRow(127, "PAGE", "Compact public preview", true);
  AppVisual::drawDataRow(151, "LINKS", "2 available");
  AppVisual::drawDataRow(175, "FORMAT", "Text-first web");
  WatchyUi::Widget::footer("SELECT OPEN LINK  BACK EXIT");
  WatchyUi::Screen::present();
}

void drawPingResult() {
  constexpr int16_t samples[] = {18, 17, 19, 18};
  WatchyUi::Screen::begin("PING");
  AppVisual::drawMetric({12, 32, 176, 62}, "AVERAGE LATENCY", "18 ms");
  AppVisual::drawMiniChart({12, 108, 176, 48}, samples,
                           sizeof(samples) / sizeof(samples[0]), 0, 40, true);
  AppVisual::drawDataRow(177, "REPLIES", "4 / 4", true);
  WatchyUi::Widget::footer("EXAMPLE.NET  BACK EXIT");
  WatchyUi::Screen::present();
}

void drawTraceResult() {
  const char *const values[] = {"2 ms", "11 ms", "17 ms", "19 ms"};
  WatchyUi::Screen::begin("TRACEROUTE");
  const uint16_t foreground = WatchyUi::Theme::foreground();
  Watchy::display.drawLine(30, 43, 30, 145, foreground);
  for (uint8_t hop = 0; hop < 4; hop++) {
    int16_t y = 48 + hop * 26;
    Watchy::display.fillCircle(30, y - 4, 4, foreground);
    char label[8];
    snprintf(label, sizeof(label), "HOP %u", hop + 1);
    AppVisual::drawDataRow(y, label, values[hop], hop == 3);
  }
  AppVisual::drawDataRow(170, "DESTINATION", "Reached", true);
  WatchyUi::Widget::footer("4 HOPS / EXAMPLE.NET  BACK EXIT");
  WatchyUi::Screen::present();
}

void drawPortResult() {
  const char *const ports[] = {"22 SSH", "80 HTTP", "443 HTTPS"};
  WatchyUi::Screen::begin("PORT SCANNER");
  AppVisual::drawMetric({12, 32, 176, 62}, "OPEN COMMON PORTS", "3");
  for (uint8_t index = 0; index < 3; index++) {
    int16_t y = 123 + index * 20;
    Watchy::display.fillCircle(21, y - 4, 4, WatchyUi::Theme::foreground());
    AppVisual::drawDataRow(y, "OPEN", ports[index], index == 0);
  }
  WatchyUi::Widget::footer("AUTHORIZED HOSTS ONLY  BACK EXIT");
  WatchyUi::Screen::present();
}

void drawDnsResult(bool reverse) {
  WatchyUi::Screen::begin(reverse ? "REVERSE DNS" : "DNS QUERY");
  AppVisual::drawStatusIcon({79, 35, 42, 42}, AppVisual::StatusIcon::RADIO,
                            true);
  AppVisual::drawMetric({12, 87, 176, 66}, reverse ? "PTR RECORD" : "A RECORD",
                        reverse ? "watchy-demo" : "192.0.2.80");
  AppVisual::drawDataRow(173, "STATUS", "NOERROR / TTL 300", true);
  WatchyUi::Widget::footer("DNS RESPONSE  BACK EXIT");
  WatchyUi::Screen::present();
}

void drawSearchResult() {
  WatchyUi::Screen::begin("DUCKDUCKGO");
  AppVisual::drawMetric({12, 32, 176, 62}, "TEXT RESULTS", "2");
  AppVisual::drawDataRow(124, "RESULT 1", "Watchy documentation", true);
  AppVisual::drawDataRow(149, "RESULT 2", "Application gallery");
  AppVisual::drawDataRow(174, "QUERY", "watchy open source");
  WatchyUi::Widget::footer("SELECT OPEN  BACK EXIT");
  WatchyUi::Screen::present();
}

void drawWifiResult() {
  const char *const names[] = {"WATCHY_DEMO", "LAB_GUEST", "OPEN_DEMO"};
  const int rssi[] = {-47, -68, -81};
  WatchyUi::Screen::begin("WIFI SURVEY");
  AppVisual::drawMetric({12, 32, 176, 58}, "NETWORKS FOUND", "3");
  for (uint8_t index = 0; index < 3; index++) {
    int16_t y = 120 + index * 20;
    AppVisual::drawDataRow(y, names[index], rssi[index] > -60 ? "Strong"
                                      : rssi[index] > -75 ? "Fair" : "Weak",
                            index == 0);
    AppVisual::drawSignalBars({145, static_cast<int16_t>(y - 12), 38, 15},
                              rssi[index] > -55 ? 4
                              : rssi[index] > -65 ? 3
                              : rssi[index] > -80 ? 2 : 1);
  }
  WatchyUi::Widget::footer("SIGNAL SORTED  BACK EXIT");
  WatchyUi::Screen::present();
}

} // namespace

void renderNetworkPreview(uint8_t tool, uint8_t view) {
  if (tool >= sizeof(titles) / sizeof(titles[0])) {
    tool = 0;
  }

  if (tool == 1) {
    if (view == 0) {
      const char *const sources[] = {
          "AFP", "ANSA", "UPI", "BBC News", "Al Jazeera", "France 24",
          "Custom URL"};
      WatchyUi::ListView::draw(
          "NEWS SOURCE", sources, sizeof(sources) / sizeof(sources[0]), 0,
          "UP/DOWN SELECT      BACK EXIT", -1,
          WatchyUi::Theme::listVisibleRows);
      WatchyUi::Screen::present();
    } else if (view == 1) {
      NetworkApps::renderTextEditorPreview(
          "RSS URL", "https://example.com/watchy.xml");
    } else if (view == 2) {
      drawNetworkLoading("RSS FEED", "Downloading feed");
    } else if (view == 3) {
      NetworkApps::FeedList items;
      items.count = 3;
      items.entries[0].title = "Watchy SDK adds deterministic previews";
      items.entries[1].title = "Low-memory RSS reader ships on e-paper";
      items.entries[2].title = "Application gallery expands to all views";
      NetworkApps::drawFeedHeadlines("RSS HEADLINES", items, 0);
    } else {
      const char *article =
          "Watchy now captures each distinct application state.\n\n"
          "This text-only article demonstrates wrapped, scrollable RSS "
          "content on the 200x200 e-paper display.";
      WatchyUi::ScrollableTextView::draw(WatchyUi::ScrollableTextModel{
          "RSS ARTICLE", article, "UP/DOWN SCROLL     BACK EXIT",
          0, 31, 16, 9});
      WatchyUi::Screen::present();
    }
    return;
  }

  if (tool == 8) {
    if (view == 0) {
      drawNetworkLoading("WIFI SURVEY", "Scanning nearby networks");
      return;
    }
    if (view >= 2) {
      drawNetworkTerminal("WIFI SURVEY", "NO NETWORKS",
                          "Scan completed with no results", true);
      return;
    }
    drawWifiResult();
    return;
  } else {
    const char *const editorValues[] = {
        "https://example.com/watchy", "", "example.net", "example.net",
        "192.0.2.80", "example.com", "192.0.2.42",
        "watchy open source", ""};
    const char *const loadingMessages[] = {
        "Downloading page...", "", "Sending ICMP echo...",
        "Tracing route...", "Scanning common ports...",
        "Resolving A record...", "Resolving PTR record...",
        "Searching DuckDuckGo...", ""};
    const char *const terminalMessages[] = {
        "Page download failed.", "", "Request timed out.",
        "Trace cancelled.", "No common ports open.",
        "DNS query failed.", "PTR lookup failed.",
        "No text results found.", ""};
    if (view == 0) {
      NetworkApps::renderTextEditorPreview(titles[tool], editorValues[tool]);
      return;
    }
    if (view == 1) {
      drawNetworkLoading(titles[tool], loadingMessages[tool]);
      return;
    }
    if (view >= 3) {
      drawNetworkTerminal(titles[tool], tool == 4 || tool == 7 ? "NO RESULTS"
                                                                : "NETWORK ERROR",
                          terminalMessages[tool], tool == 4 || tool == 7);
      return;
    }
  }

  switch (tool) {
  case 0: drawBrowserResult(); break;
  case 2: drawPingResult(); break;
  case 3: drawTraceResult(); break;
  case 4: drawPortResult(); break;
  case 5: drawDnsResult(false); break;
  case 6: drawDnsResult(true); break;
  case 7: drawSearchResult(); break;
  default:
    break;
  }
}

} // namespace WatchyDemo

#endif
