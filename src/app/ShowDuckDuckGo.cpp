#include "NetworkAppCommon.h"

void Watchy::showDuckDuckGo() {
  String query;
  if (!NetworkApps::editText("DUCKDUCKGO", "esp32", query)) {
    showMenu(menuIndex, false);
    return;
  }
  if (!NetworkApps::ensureConnected(*this, "DUCKDUCKGO")) {
    showMenu(menuIndex, false);
    return;
  }

  String searchUrl = "https://lite.duckduckgo.com/lite/?q=" +
                     NetworkApps::urlEncode(query);
  NetworkApps::showStatus("DUCKDUCKGO", "Searching...");
  NetworkApps::LinkList results;
  bool httpsFailed = false;
  {
    String html;
    String contentType;
    NetworkApps::LinkList allLinks;
    if (NetworkApps::fetchText(searchUrl, html, contentType)) {
      NetworkApps::extractLinks(html, searchUrl, allLinks);
      for (uint8_t index = 0;
           index < allLinks.count && results.count < NetworkApps::maximumLinks;
           index++) {
        String lowerUrl = allLinks.entries[index].url;
        lowerUrl.toLowerCase();
        if (lowerUrl.indexOf("duckduckgo.com") < 0) {
          results.entries[results.count++] = allLinks.entries[index];
        }
      }
    } else {
      httpsFailed = contentType == "error/https";
    }
  }
  NetworkApps::disconnect();

  if (results.count == 0) {
    NetworkApps::showStatus(
        "DUCKDUCKGO",
        httpsFailed
            ? "HTTPS/TLS failed\nCheck clock/certificate\n\nBACK to return"
            : "No results found\n\nBACK to return");
    while (NetworkApps::waitForButton() != NetworkApps::BUTTON_BACK) {}
  } else {
    uint8_t selected = 0;
    while (true) {
      int choice = NetworkApps::chooseLink("SEARCH RESULTS", results, selected);
      if (choice < 0) break;
      selected = choice;
      NetworkApps::browse(*this, results.entries[choice].url);
    }
  }

  NetworkApps::disconnect();
  showMenu(menuIndex, false);
}