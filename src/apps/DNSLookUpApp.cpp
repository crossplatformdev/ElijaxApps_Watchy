#include "../watchy/Watchy.h"
#include "../sdk/UiSDK.h"
#include "../sdk/UiTemplates.h"
#include "../sdk/NetUtils.h"
#include "NetAppCommon.h"
#include <cstring>

// Dig rebuilt to match Ping UX: history, uniform target editor (space-first, acceptx2 to finish), scrollable results.

RTC_DATA_ATTR uint16_t digLastQtype = 1; // A

struct DigQTypeItem {
  const char *label;
  uint16_t qtype;
};

static const DigQTypeItem DIG_QTYPES[] = {
    {"A", 1},
    {"AAAA", 28},
    {"MX", 15},
    {"TXT", 16},
    {"NS", 2},
};

static uint8_t qtypeIndex(uint16_t qtype) {
  for (uint8_t i = 0; i < (sizeof(DIG_QTYPES) / sizeof(DIG_QTYPES[0])); ++i) {
    if (DIG_QTYPES[i].qtype == qtype) {
      return i;
    }
  }
  return 0;
}

static bool pickDigQtype(Watchy &watchy, uint16_t &inOutQtype) {
  int8_t selected = qtypeIndex(inOutQtype);
  const uint8_t total = sizeof(DIG_QTYPES) / sizeof(DIG_QTYPES[0]);

  static UIMenuItemSpec items[5];
  for (uint8_t i = 0; i < total; ++i) {
    items[i].label = DIG_QTYPES[i].label;
  }

  UiTemplates::MenuPickerLayout layout;
  const int8_t chosen = UiTemplates::runMenuPicker(
      watchy,
      "DNS Type",
      items,
      total,
      selected,
      layout,
      "BACK",
      "UP",
      "ACCEPT",
      "DOWN");
  if (chosen < 0) return false;

  inOutQtype = DIG_QTYPES[chosen].qtype;
  return true;
}

void Watchy::showDig() {
  guiState = APP_STATE;

  static int8_t sHistorySelected = -1;

  while (true) {
    char selectedTarget[UiTemplates::HISTORY_MAX_ENTRY_LEN + 1] = {0};
    if (!NetAppCommon::pickTargetEditAndPersist(*this, "hist_dig", "Dig target", sHistorySelected, selectedTarget, sizeof(selectedTarget))) {
      showMenu(menuIndex);
      return;
    }

    const char *host = selectedTarget;

    String body;
    body = String("DNS ") + host + "\nQuerying...";

    UIScrollableTextSpec scroll{};
    scroll.x = 0;
    scroll.y = 18;
    scroll.w = WatchyDisplay::WIDTH;
    scroll.h = WatchyDisplay::HEIGHT - 36;
    scroll.font = UiSDK::tinyMono6x8();
    scroll.fillBackground = false;
    scroll.textRef = &body;
    scroll.firstLine = 0;
    scroll.lineHeight = 10;
    scroll.centered = false;
    scroll.wrapLongLines = true;
    scroll.wrapContinuationAlignRight = true;

    // Match TextBrowser-style visible line computation.
    const int16_t padding = 6;
    const int16_t contentH = scroll.h - padding * 2;
    uint8_t visibleLines = (contentH > 0) ? static_cast<uint8_t>(contentH / scroll.lineHeight) : 1;
    if (visibleLines < 1) visibleLines = 1;
    scroll.maxLines = visibleLines;

    UIAppSpec app{};
    app.texts = nullptr;
    app.textCount = 0;
    app.scrollTexts = &scroll;
    app.scrollTextCount = 1;

    UiTemplates::renderScrollablePage(*this, app, "EXIT", "UP", "-", "DOWN");

    if (!NetUtils::ensureWiFi(*this, 1, 10000)) {
      body = "WiFi not connected";
    } else {
      String out;
      out.reserve(6144);
      for (uint8_t i = 0; i < (sizeof(DIG_QTYPES) / sizeof(DIG_QTYPES[0])); ++i) {
        out += "=== ";
        out += DIG_QTYPES[i].label;
        out += " ===\n";
        out += NetUtils::dnsLookup(host, DIG_QTYPES[i].qtype);
        out += "\n";

        // Keep UI responsive during multi-query runs.
        delay(0);
      }
      body = out;
    }

    uint16_t firstLine = 0;
    const UiTemplates::ViewerAction action =
        UiTemplates::runScrollableViewer(*this, app, scroll, firstLine, "EXIT", "UP", "TARGETS", "DOWN", true);

    if (action == UiTemplates::ViewerAction::Accept) {
      continue; // back to target selector
    }

    showMenu(menuIndex);
    return;
  }
}
