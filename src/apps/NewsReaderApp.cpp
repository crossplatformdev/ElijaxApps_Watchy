#include "../watchy/Watchy.h"
#include "../sdk/UiSDK.h"
#include "../sdk/NetUtils.h"
#include "../sdk/UiTemplates.h"

// News app: opens the TextBrowser on a curated set of lightweight sources.
// Some endpoints change over time or are too heavy for an on-device HTML-to-text
// renderer, so we normalize certain URLs to known-working alternatives.

namespace {

struct NewsGroup {
  const char *title;
  const UIMenuItemSpec *items;
  const char *const *urls;
  uint8_t count;
};

// Top-level categories.
static const UIMenuItemSpec kNewsCategories[] = {
  {"US/World"},
  {"Canada"},
  {"UK"},
  {"Tech"},
  {"Science"},
  {"Sports"},
  {"Markets"},
  {"Wikipedia"},
  {"Weather"},
  {"Alt Frontends"},
};

// US/World
static const UIMenuItemSpec kUsWorldItems[] = {
  {"Kagi News"},
  {"Brutalist Report"},
  {"CNN Lite"},
  {"Reuters"},
};
static const char *const kUsWorldUrls[] = {
  "https://kagi.com/news",
  "https://brutalist.report/",
  "https://lite.cnn.com/",
  "https://www.reuters.com/",
};

// Canada
static const UIMenuItemSpec kCanadaItems[] = {
  {"CBC Lite"},
};
static const char *const kCanadaUrls[] = {
  "https://www.cbc.ca/lite/",
};

// UK
static const UIMenuItemSpec kUkItems[] = {
  {"Guardian (minimal)"},
};
static const char *const kUkUrls[] = {
  "https://guardian.gyford.com/",
};

// Tech
static const UIMenuItemSpec kTechItems[] = {
  {"Hacker News"},
  {"Hckr News"},
  {"Lobsters"},
  {"AlterSlash"},
  {"68k News"},
  {"1MB Club"},
};
static const char *const kTechUrls[] = {
  "https://news.ycombinator.com/",
  "https://hckrnews.com/",
  "https://lobste.rs/",
  "https://alterslash.org/",
  "http://68k.news/",
  "https://1mb.club/",
};

// Science
static const UIMenuItemSpec kScienceItems[] = {
  {"ScienceDaily"},
};
static const char *const kScienceUrls[] = {
  "https://www.sciencedaily.com/",
};

// Sports
static const UIMenuItemSpec kSportsItems[] = {
  {"PlainTextSports"},
};
static const char *const kSportsUrls[] = {
  "https://plaintextsports.com/",
};

// Markets
static const UIMenuItemSpec kMarketsItems[] = {
  {"Markets.sh"},
};
static const char *const kMarketsUrls[] = {
  "https://markets.sh/l/news",
};

// Wikipedia
static const UIMenuItemSpec kWikiItems[] = {
  {"Main Page"},
  {"Current Events"},
};
static const char *const kWikiUrls[] = {
  "https://en.wikipedia.org/wiki/Main_Page?action=render",
  "https://en.wikipedia.org/wiki/Portal:Current_events?action=render",
};

// Weather
static const UIMenuItemSpec kWeatherItems[] = {
  {"wttr.in"},
};
static const char *const kWeatherUrls[] = {
  "https://wttr.in/",
};

// Alternate/lightweight frontends (some are not strictly "news", but are useful
// on text-first devices).
static const UIMenuItemSpec kAltFrontendItems[] = {
  {"NPR Text"},
  {"SVT Text"},
  {"Scribe (Medium)"},
  {"Xcancel (X/Twitter)"},
};
static const char *const kAltFrontendUrls[] = {
  "https://text.npr.org/",
  "https://www.svt.se/text-tv/webb/100",
  "https://scribe.rip/",
  "https://xcancel.com/",
};

static const NewsGroup kNewsGroups[] = {
  {"US/World", kUsWorldItems, kUsWorldUrls, static_cast<uint8_t>(sizeof(kUsWorldItems) / sizeof(kUsWorldItems[0]))},
  {"Canada", kCanadaItems, kCanadaUrls, static_cast<uint8_t>(sizeof(kCanadaItems) / sizeof(kCanadaItems[0]))},
  {"UK", kUkItems, kUkUrls, static_cast<uint8_t>(sizeof(kUkItems) / sizeof(kUkItems[0]))},
  {"Tech", kTechItems, kTechUrls, static_cast<uint8_t>(sizeof(kTechItems) / sizeof(kTechItems[0]))},
  {"Science", kScienceItems, kScienceUrls, static_cast<uint8_t>(sizeof(kScienceItems) / sizeof(kScienceItems[0]))},
  {"Sports", kSportsItems, kSportsUrls, static_cast<uint8_t>(sizeof(kSportsItems) / sizeof(kSportsItems[0]))},
  {"Markets", kMarketsItems, kMarketsUrls, static_cast<uint8_t>(sizeof(kMarketsItems) / sizeof(kMarketsItems[0]))},
  {"Wikipedia", kWikiItems, kWikiUrls, static_cast<uint8_t>(sizeof(kWikiItems) / sizeof(kWikiItems[0]))},
  {"Weather", kWeatherItems, kWeatherUrls, static_cast<uint8_t>(sizeof(kWeatherItems) / sizeof(kWeatherItems[0]))},
  {"Alt Frontends", kAltFrontendItems, kAltFrontendUrls, static_cast<uint8_t>(sizeof(kAltFrontendItems) / sizeof(kAltFrontendItems[0]))},
};

static constexpr uint8_t kNewsGroupCount = static_cast<uint8_t>(sizeof(kNewsGroups) / sizeof(kNewsGroups[0]));

static const NewsGroup *groupFor(uint8_t idx) {
  return (idx < kNewsGroupCount) ? &kNewsGroups[idx] : nullptr;
}

} // namespace

static String normalizeNewsUrl(const String &url) {
  // Wikipedia: use action=render for a smaller, simpler HTML payload.
  if (url.startsWith("https://en.wikipedia.org/wiki/") || url == "https://en.wikipedia.org/wiki/") {
    return "https://en.wikipedia.org/wiki/Main_Page?action=render";
  }

  // SVT: /svttext currently returns 404; teletext lives under /text-tv.
  if (url.startsWith("https://www.svt.se/svttext")) {
    // Use the "webb" view to avoid very large image/base64 payloads.
    return "https://www.svt.se/text-tv/webb/100";
  }

  // markets.sh: /news currently serves a 404 page; the marketing route exists.
  if (url.startsWith("https://markets.sh/news")) {
    return "https://markets.sh/l/news";
  }

  // PlainTextSports prefers HTTPS.
  if (url.startsWith("http://plaintextsports.com")) {
    return String("https://plaintextsports.com/");
  }

  return url;
}

void Watchy::showNewsReader() {
  guiState = APP_STATE;

  int8_t selectedCategory = 0;
  int8_t selectedSource = 0;

  UiTemplates::MenuPickerLayout layout;
  layout.headerX = 0;
  layout.headerY = 36;
  layout.menuX = 0;
  layout.menuY = 72;
  layout.visibleRowsMax = 5;
  layout.autoScroll = true;
  layout.font = &FreeMonoBold9pt7b;

  while (true) {
    const int8_t catRes = UiTemplates::runMenuPicker(*this,
                                                    "News",
                                                    kNewsCategories,
                                                    kNewsGroupCount,
                                                    selectedCategory,
                                                    layout,
                                                    "BACK",
                                                    "UP",
                                                    "ENTER",
                                                    "DOWN");
    if (catRes < 0) {
      showMenu(menuIndex);
      return;
    }

    const NewsGroup *g = groupFor(static_cast<uint8_t>(catRes));
    if (g == nullptr || g->count == 0) {
      continue;
    }

    if (selectedSource < 0) {
      selectedSource = 0;
    }
    if (selectedSource >= static_cast<int8_t>(g->count)) {
      selectedSource = static_cast<int8_t>(g->count - 1);
    }

    const int8_t srcRes = UiTemplates::runMenuPicker(*this,
                                                    g->title,
                                                    g->items,
                                                    g->count,
                                                    selectedSource,
                                                    layout,
                                                    "BACK",
                                                    "UP",
                                                    "OPEN",
                                                    "DOWN");
    if (srcRes < 0) {
      continue; // back to categories
    }

    showTextBrowser(normalizeNewsUrl(String(g->urls[srcRes])));
    return;
  }
}
