#include "../watchy/Watchy.h"
#include "../sdk/UiSDK.h"
#include "../sdk/NetUtils.h"
#include "../sdk/UiTemplates.h"

// Minimal HTTP client with preset requests (GET-only, safe for device constraints).

struct RequestPreset {
  String label;
  String url;
};

static RequestPreset kRequests[] = {
  {"GET httpbin/get", "https://httpbin.org/get"},
  {"GET httpbin/uuid", "https://httpbin.org/uuid"},
  {"GET ipify", "https://api.ipify.org?format=json"},
  {"GET jsonplaceholder", "https://jsonplaceholder.typicode.com/todos/1"},
  {"GET example.com", "https://example.com"},
};
static const uint8_t kRequestCount = sizeof(kRequests) / sizeof(kRequests[0]);

void Watchy::showPostman() {
  guiState = APP_STATE;

  int8_t selected = 0;

  UiTemplates::MenuPickerLayout layout;
  layout.headerX = 0;
  layout.headerY = 36;
  layout.menuX = 0;
  layout.menuY = 72;
  layout.visibleRowsMax = 4;
  layout.startIndex = 0;
  layout.autoScroll = false;
  layout.font = &FreeMonoBold9pt7b;

  static UIMenuItemSpec items[kRequestCount];
  for (uint8_t i = 0; i < kRequestCount; ++i) {
    items[i].label = kRequests[i].label;
  }

  while (true) {
    const int8_t chosen = UiTemplates::runMenuPicker(*this,
                                                     "HTTP client",
                                                     items,
                                                     kRequestCount,
                                                     selected,
                                                     layout,
                                                     "BACK",
                                                     "UP",
                                                     "SEND",
                                                     "DOWN");
    if (chosen < 0) {
      showMenu(menuIndex);
      return;
    }

    // Execute selected request
    UITextSpec reqHeader{};
    reqHeader.x = 16;
    reqHeader.y = 36;
    reqHeader.font = &FreeMonoBold9pt7b;
    reqHeader.text = kRequests[chosen].label;

    UIScrollableTextSpec scroll{};
    scroll.x = 0;
    scroll.y = 56;
    scroll.w = WatchyDisplay::WIDTH;
    scroll.h = WatchyDisplay::HEIGHT - 80; // leave space for control row
    scroll.font = nullptr;
    scroll.fillBackground = false;
    scroll.text = "Sending...";
    scroll.textRef = nullptr;
    scroll.firstLine = 0;
    scroll.maxLines = 14;
    scroll.lineHeight = 8;
    scroll.centered = false;
    scroll.wrapLongLines = false;
    scroll.wrapContinuationAlignRight = false;

    UIAppSpec reqApp{};
    reqApp.texts = &reqHeader;
    reqApp.textCount = 1;
    reqApp.scrollTexts = &scroll;
    reqApp.scrollTextCount = 1;

    UiTemplates::renderScrollablePage(*this, reqApp, "BACK", "UP", "-", "DOWN");

    String body;
    body.reserve(512);

    if (!NetUtils::ensureWiFi(*this, 1, 8000)) {
      body = "WiFi not connected";
    } else {
      NetResponse resp = NetUtils::httpGet(kRequests[chosen].url, 8000, false);
      if (resp.httpCode == 200) {
        body = resp.body;
      } else {
        body = (resp.error.length() > 0) ? resp.error : String("HTTP error: ") + resp.httpCode;
      }
    }

    scroll.text = "";
    scroll.textRef = &body;
    uint16_t firstLine = 0;

    UiTemplates::runScrollableViewer(*this, reqApp, scroll, firstLine, "BACK", "UP", "-", "DOWN", false);
    // BACK returns to presets
  }
}
