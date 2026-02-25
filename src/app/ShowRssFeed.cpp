#include "NetworkAppCommon.h"


#include "WatchyUi.h"
#include "Watchy.h"

namespace {

constexpr size_t maximumArticleBytes = 6144;

struct NewsSource {
  const char *name;
  const char *url;
};

constexpr NewsSource newsSources[] = {
    {"AFP", "https://www.afp.com/en/rss.xml"},
    {"ANSA", "https://www.ansa.it/sito/ansait_rss.xml"},
    {"UPI", "https://www.upi.com/rss/news.rss"},
    {"BBC News", "https://feeds.bbci.co.uk/news/rss.xml"},
    {"Al Jazeera", "https://www.aljazeera.com/xml/rss/all.xml"},
    {"France 24", "https://www.france24.com/en/rss"},
    {"Custom URL", nullptr},
};

constexpr uint8_t newsSourceCount =
    sizeof(newsSources) / sizeof(newsSources[0]);

const char *newsSourceLabel(uint8_t index, const void *) {
  return index < newsSourceCount ? newsSources[index].name : "";
}

void drawNewsSources(uint8_t selected) {
  WatchyUi::ListModel model{
      "NEWS SOURCE", newsSourceLabel, nullptr, nullptr,
      "UP/DOWN SELECT      BACK EXIT", newsSourceCount, selected,
      WatchyUi::Theme::listVisibleRows, -1, true, false};
  WatchyUi::ListView::draw(model);
  WatchyUi::Screen::present();
}

void updateNewsSources(uint8_t previous, uint8_t selected) {
  WatchyUi::ListModel model{
      "NEWS SOURCE", newsSourceLabel, nullptr, nullptr,
      "UP/DOWN SELECT      BACK EXIT", newsSourceCount, selected,
      WatchyUi::Theme::listVisibleRows, -1, true, false};
  WatchyUi::ListView::presentSelectionChange(model, previous);
}

bool chooseNewsSource(String &url) {
  uint8_t selected = 0;
  WatchyUi::Input::begin();
  drawNewsSources(selected);
  while (true) {
    WatchyUi::Event event = WatchyUi::Input::wait();
    if (event == WatchyUi::Event::BACK) return false;
    if (event == WatchyUi::Event::UP) {
      uint8_t previous = selected;
      selected = WatchyUi::ListView::previous(selected, newsSourceCount);
      updateNewsSources(previous, selected);
    } else if (event == WatchyUi::Event::DOWN) {
      uint8_t previous = selected;
      selected = WatchyUi::ListView::next(selected, newsSourceCount);
      updateNewsSources(previous, selected);
    } else if (event == WatchyUi::Event::MENU) {
      const char *preset = newsSources[selected].url;
      if (preset != nullptr) {
        url = preset;
        return true;
      }
      String customUrl;
      if (NetworkApps::editText("RSS URL", NETWORKING_RSS_FEED_CUSTOM_URL,
                customUrl,
                                NetworkApps::maximumUrlLength)) {
        url = customUrl;
        return true;
      }
      WatchyUi::Input::begin();
      drawNewsSources(selected);
    }
  }
}

void showArticle(const NetworkApps::FeedItem &item, Watchy *watchy) {
  if (item.content.length() > 0) {
    WatchyUi::ScrollableTextView::show(
        "RSS ARTICLE", item.content.c_str());
    return;
  }
  if (item.url.length() == 0) {
    WatchyUi::ScrollableTextView::show(
        "RSS ARTICLE", "No article content was supplied.");
    return;
  }

  if (!NetworkApps::ensureConnected("RSS ARTICLE", watchy)) {
    return;
  }
  NetworkApps::showStatus("RSS ARTICLE", "Downloading article...");
  String resource;
  String contentType;
  bool fetched = NetworkApps::fetchText(
      item.url, resource, contentType, maximumArticleBytes);
  NetworkApps::disconnect();
  if (!fetched) {
    WatchyUi::ScrollableTextView::show(
        "RSS ARTICLE", "Text-only article download failed.");
    return;
  }

  String lowerType = contentType;
  lowerType.toLowerCase();
  if (lowerType.indexOf("html") >= 0 || resource.indexOf('<') >= 0) {
    NetworkApps::htmlToTextInPlace(resource, maximumArticleBytes);
  }
  resource.trim();
  WatchyUi::ScrollableTextView::show(
      "RSS ARTICLE", resource.length() > 0
                         ? resource.c_str() : "Article content is empty.");
}

} // namespace

void showRssFeedImpl(Watchy *watchy) {
  String url;
  if (!chooseNewsSource(url)) {
    if (watchy != nullptr) watchy->showMenu(menuIndex, false);
    else WatchySdk::showMenu(menuIndex, false);
    return;
  }
  if (!NetworkApps::ensureConnected("RSS FEED", watchy)) {
    if (watchy != nullptr) watchy->showMenu(menuIndex, false);
    else WatchySdk::showMenu(menuIndex, false);
    return;
  }

  NetworkApps::showStatus("RSS FEED", "Downloading feed...");
  NetworkApps::FeedList items;
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
      int choice = NetworkApps::chooseFeedItem("RSS HEADLINES", items,
                                               selected);
      if (choice < 0) break;
      selected = choice;
      showArticle(items.entries[choice], watchy);
    }
  }

  NetworkApps::disconnect();
  if (watchy != nullptr) watchy->showMenu(menuIndex, false);
  else WatchySdk::showMenu(menuIndex, false);
}

void Watchy::showRssFeed() { showRssFeedImpl(this); }

void WatchySdk::showRssFeed() { showRssFeedImpl(nullptr); }
