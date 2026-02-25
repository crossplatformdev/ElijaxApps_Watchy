#include "NetworkAppCommon.h"

void Watchy::showBrowser() {
  String url;
  if (NetworkApps::editText("BROWSER URL", "https://example.com", url,
                            NetworkApps::maximumUrlLength)) {
    if (NetworkApps::ensureConnected(*this, "BROWSER")) {
      NetworkApps::browse(*this, url);
    }
    NetworkApps::disconnect();
  }
  showMenu(menuIndex, false);
}