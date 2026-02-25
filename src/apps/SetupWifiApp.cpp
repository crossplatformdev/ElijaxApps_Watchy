#include "./../watchy/Watchy.h"
#include "./../sdk/UiSDK.h"
#include "./../sdk/UiTemplates.h"

void renderStatus(Watchy &watchy, const char *l1, const char *l2 = nullptr,
                  bool /*partial*/ = false) {
  UiSDK::initScreen(watchy.display);
  watchy.display.display(true); // ensure any previous content is fully rendered before drawing status
  const char *lines[2] = {l1, l2};
  UiTemplates::renderStatusLines(
    watchy,
    lines,
    2,
    /*x=*/10,
    /*y=*/40,
    /*lineSpacing=*/26,
    UiSDK::defaultFont(),
    "BACK"
  );
  watchy.display.display(true);
}

void Watchy::setupWifi() {
  display.epd2.setBusyCallback(0); // temporarily disable lightsleep on busy

  renderStatus(*this, "WiFi Setup", "Starting AP...");
  WiFiManager wifiManager;
  const bool ok = wifiManager.autoConnect(lastSSID, lastPassword);

  if (!ok) {
    wifiManager.resetSettings();
    wifiManager.setAPCallback(_configModeCallback);
    wifiManager.setTimeout(WIFI_AP_TIMEOUT);
    wifiManager.startConfigPortal(WIFI_AP_SSID); // If we get here, the timeout expired while trying to connect to the AP, or // we failed to start the AP. In either case, show an error and return to // the menu. renderStatus(display, "WiFi Setup Failed", "Returning to menu..."); delay(2000); showMenu(menuIndex); return;
  } else {
    char ipBuf[32];
    snprintf(ipBuf, sizeof(ipBuf), "%s", WiFi.localIP().toString().c_str());
    UiSDK::initScreen(display); 
    display.display(true);
    
    weatherIntervalCounter = -1; // force weather refresh
    lastIPAddress = WiFi.localIP();
    WiFi.SSID().toCharArray(lastSSID, 30);
    WiFi.psk().toCharArray(lastPassword, 64);
    
    renderStatus(*this, "WiFi Connected!", ipBuf); 
    display.display(true); 
  }

  // enable lightsleep on busy
  //display.epd2.setBusyCallback(WatchyDisplay::busyCallback);
  guiState = APP_STATE;
}
