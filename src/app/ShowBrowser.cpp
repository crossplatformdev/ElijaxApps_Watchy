#include "WatchyUi.h"
#include "Watchy.h"
#include "NetworkAppCommon.h"

void showBrowserImpl(Watchy *watchy) {
  String url;
  if (NetworkApps::editText("BROWSER URL", NETWORKING_BROWSER_URL, url,
                            NetworkApps::maximumUrlLength)) {
    if (NetworkApps::ensureConnected("BROWSER", watchy)) {
      NetworkApps::browse(url, watchy);
    }
    NetworkApps::disconnect();
  }
  if (watchy != nullptr) watchy->showMenu(menuIndex, false);
  else WatchySdk::showMenu(menuIndex, false);
}

void Watchy::showBrowser() { showBrowserImpl(this); }

void WatchySdk::showBrowser() { showBrowserImpl(nullptr); }
