#include "../watchy/Watchy.h"
#include "../sdk/UiSDK.h"
#include "../sdk/UiTemplates.h"
#include "../sdk/NetUtils.h"
#include "NetAppCommon.h"
#include <cstring>

void Watchy::showTraceroute() {
  guiState = APP_STATE;

  static int8_t sHistorySelected = -1;

  while (true) {
    char selectedTarget[UiTemplates::HISTORY_MAX_ENTRY_LEN + 1] = {0};
    if (!NetAppCommon::pickTargetEditAndPersist(*this, "hist_trace", "Traceroute target", sHistorySelected, selectedTarget, sizeof(selectedTarget))) {
      showMenu(menuIndex);
      return;
    }

    const char *host = selectedTarget;

    String body;
    body = String("Traceroute ") + host + "\nRunning...";

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
      body = NetUtils::traceroute(host);
    }

    uint16_t firstLine = 0;
    const UiTemplates::ViewerAction action =
        UiTemplates::runScrollableViewer(*this, app, scroll, firstLine, "EXIT", "UP", "TARGETS", "DOWN", true);
    if (action == UiTemplates::ViewerAction::Accept) {
      continue;
    }

    showMenu(menuIndex);
    return;
  }
}
