#include "Watchy.h"
#include "esp_wifi.h"
#include "sdk/WatchyUi.h"

extern RTC_DATA_ATTR bool WIFI_CONFIGURED;
extern RTC_DATA_ATTR uint32_t lastIPAddress;
extern RTC_DATA_ATTR char lastSSID[33];
extern RTC_DATA_ATTR long gmtOffset;
extern RTC_DATA_ATTR int weatherIntervalCounter;

namespace {

void rememberWiFiConnection() {
  lastIPAddress = WiFi.localIP();
  WiFi.SSID().toCharArray(lastSSID, sizeof(lastSSID));
  WIFI_CONFIGURED = true;
  esp_wifi_set_ps(WIFI_PS_MAX_MODEM);
}

void turnOffWiFi() {
  WiFi.disconnect(true, false);
  btStop();
  Watchy::setLowPowerCpuFrequency();
}

bool connectSavedWiFi(bool allowBack, bool &cancelled) {
  Watchy::setRadioCpuFrequency();
  cancelled = false;
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.mode(WIFI_STA);
    if (WiFi.begin() != WL_CONNECT_FAILED) {
      uint32_t startedAt = millis();
      while (WiFi.status() != WL_CONNECTED &&
             millis() - startedAt < WIFI_CONNECT_TIMEOUT * 1000UL) {
        if (allowBack &&
          WatchyUi::Input::pressed(WatchyUi::Event::BACK)) {
          cancelled = true;
          break;
        }
        delay(20);
      }
    }
  }

  if (WiFi.status() == WL_CONNECTED) {
    rememberWiFiConnection();
    return true;
  }
  WIFI_CONFIGURED = false;
  return false;
}

void beginWiFiScreen() {
  Watchy::display.setFullWindow();
  Watchy::display.fillScreen(DARKMODE ? GxEPD_BLACK : GxEPD_WHITE);
  Watchy::display.setFont(&FreeMonoBold9pt7b);
  Watchy::display.setTextColor(DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
  Watchy::display.setCursor(0, 24);
}

} // namespace

bool Watchy::connectWiFi() {
  bool cancelled = false;
  bool connected = connectSavedWiFi(false, cancelled);
  if (!connected) turnOffWiFi();
  return connected;
}

void Watchy::setupWifi() {
  display.epd2.setBusyCallback(0); // temporarily disable lightsleep on busy
  bool returnToMenu = guiState == MAIN_MENU_STATE;
  guiState = APP_STATE;
  WatchyUi::Input::begin();

  beginWiFiScreen();
  display.println("Connecting to");
  display.println("saved WiFi...");
  display.println();
  display.println("BACK to cancel");
  display.display(true);

  bool cancelled = false;
  bool connected = connectSavedWiFi(true, cancelled);
  bool timedOut = false;
  bool portalFailed = false;

  if (!connected && !cancelled) {
    WiFiManager wifiManager;
    wifiManager.setConfigPortalBlocking(false);
    wifiManager.setConnectTimeout(3);
    wifiManager.setSaveConnectTimeout(3);
    wifiManager.setConnectRetries(1);
    wifiManager.setAPStaticIPConfig(IPAddress(192, 168, 4, 1),
                                    IPAddress(192, 168, 4, 1),
                                    IPAddress(255, 255, 255, 0));
    wifiManager.setCaptivePortalEnable(true);
    wifiManager.setAPCallback(_configModeCallback);
    wifiManager.startConfigPortal(WIFI_AP_SSID);

    if (!wifiManager.getConfigPortalActive()) {
      portalFailed = true;
    } else {
      uint32_t unattendedSince = millis();
      while (wifiManager.getConfigPortalActive() && !connected) {
        if (WatchyUi::Input::pressed(WatchyUi::Event::BACK)) {
          cancelled = true;
          wifiManager.stopConfigPortal();
          break;
        }

        connected = wifiManager.process() || WiFi.status() == WL_CONNECTED;
        if (WiFi.softAPgetStationNum() > 0) {
          unattendedSince = millis();
        } else if (millis() - unattendedSince >=
                   WIFI_AP_TIMEOUT * 1000UL) {
          timedOut = true;
          wifiManager.stopConfigPortal();
          break;
        }
        delay(10);
      }
      if (connected && wifiManager.getConfigPortalActive()) {
        wifiManager.stopConfigPortal();
      }
    }
  }

  beginWiFiScreen();
  if (connected) {
    rememberWiFiConnection();
    display.println("Connected to:");
    display.println(WiFi.SSID());
    display.println("Local IP:");
    display.println(WiFi.localIP());
    weatherIntervalCounter = -1; // Reset to force weather to be read again
  } else if (cancelled) {
    display.println("WiFi setup");
    display.println("cancelled");
  } else if (timedOut) {
    display.println("Watchy AP");
    display.println("timed out");
  } else if (portalFailed) {
    display.println("Could not start");
    display.println("Watchy AP");
  } else {
    display.println("WiFi setup");
    display.println("failed");
  }
  display.display(true); // partial refresh
  turnOffWiFi();
  display.epd2.setBusyCallback(WatchyDisplay::busyCallback);
  if (cancelled && returnToMenu) {
    WatchyUi::Input::waitForRelease(WatchyUi::Event::BACK);
    showMenu(menuIndex, false);
  }
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
namespace WatchyDemo {

void renderWifiSetupPreview() {
  beginWiFiScreen();
  Watchy::display.println("Connected to:");
  Watchy::display.println("WATCHY_DEMO");
  Watchy::display.println("Local IP:");
  Watchy::display.println("192.0.2.42");
  Watchy::display.display(true);
}

} // namespace WatchyDemo
#endif