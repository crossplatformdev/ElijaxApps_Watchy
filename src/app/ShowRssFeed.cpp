#include "NetworkAppCommon.h"

void Watchy::showRssFeed() {
  String url;
  if (!NetworkApps::editText("RSS URL",
                             "https://feeds.bbci.co.uk/news/rss.xml", url,
                             NetworkApps::maximumUrlLength)) {
    showMenu(menuIndex, false);
    return;
  }
  if (!NetworkApps::ensureConnected(*this, "RSS FEED")) {
    showMenu(menuIndex, false);
    return;
  }

  NetworkApps::showStatus("RSS FEED", "Downloading feed...");
  NetworkApps::LinkList items;
  bool httpsFailed = false;
  {
    String xml;
    String contentType;
    if (NetworkApps::fetchText(url, xml, contentType)) {
      NetworkApps::extractFeed(xml, NetworkApps::normalizeUrl(url), items);
    } else {
      httpsFailed = contentType == "error/https";
    }
  }
  NetworkApps::disconnect();

  if (items.count == 0) {
    NetworkApps::showStatus(
        "RSS FEED",
        httpsFailed
            ? "HTTPS/TLS failed\nCheck clock/certificate\n\nBACK to return"
            : "No feed items found\n\nBACK to return");
    while (NetworkApps::waitForButton() != NetworkApps::BUTTON_BACK) {}
  } else {
    uint8_t selected = 0;
    while (true) {
      int choice = NetworkApps::chooseLink("RSS FEED", items, selected);
      if (choice < 0) break;
      selected = choice;
      NetworkApps::browse(*this, items.entries[choice].url);
    }
  }

  NetworkApps::disconnect();
  showMenu(menuIndex, false);
}