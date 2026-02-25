#include "WatchyUi.h"
#include "AppDisplay.h"
#include "esp_wifi.h"
#include "WatchyPowerDiagnostics.h"
#include "Watchy.h"

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
  WatchyDiagnostics::recordHeapCheckpoint(
      WatchyDiagnostics::HeapCheckpoint::Connected);
}

void turnOffWiFi() {
  WiFi.disconnect(true, false);
  btStop();
  WatchyDiagnostics::endWifiSession();
  Watchy::setLowPowerCpuFrequency();
}

bool connectSavedWiFi(bool allowBack, bool &cancelled) {
  WatchyDiagnostics::beginWifiSession();
  Watchy::setRadioCpuFrequency();
  cancelled = false;
  if (WiFi.status() != WL_CONNECTED) {
    TaskHandle_t waitingTask = xTaskGetCurrentTaskHandle();
    wifi_event_id_t connectionEvent = WiFi.onEvent(
        [waitingTask](arduino_event_id_t event,
                      arduino_event_info_t) {
          if (event == ARDUINO_EVENT_WIFI_STA_GOT_IP ||
              event == ARDUINO_EVENT_WIFI_STA_DISCONNECTED) {
            xTaskNotifyGive(waitingTask);
          }
        });
    WiFi.mode(WIFI_STA);
    if (WiFi.begin() != WL_CONNECT_FAILED) {
      uint32_t startedAt = millis();
      while (WiFi.status() != WL_CONNECTED &&
             millis() - startedAt < WIFI_CONNECT_TIMEOUT * 1000UL) {
        uint32_t remaining = WIFI_CONNECT_TIMEOUT * 1000UL -
                             (millis() - startedAt);
        WatchyUi::Event event =
          WatchyUi::Input::waitNotified(remaining);
        if (allowBack && event == WatchyUi::Event::BACK) {
          cancelled = true;
          break;
        }
      }
    }
    WiFi.removeEvent(connectionEvent);
  }

  if (WiFi.status() == WL_CONNECTED) {
    rememberWiFiConnection();
    return true;
  }
  WIFI_CONFIGURED = false;
  return false;
}

enum class WifiSetupState : uint8_t {
  CONNECTING,
  CONNECTED,
  CANCELLED,
  TIMED_OUT,
  FAILED
};

void drawWifiSetupState(WifiSetupState state, const char *ssid = nullptr,
                        const char *ip = nullptr) {
  WatchyUi::Screen::beginCanvas();
  Watchy::display.setFont();
  Watchy::display.setTextSize(1);
  WatchyUi::Canvas::centeredText({0, 1, DISPLAY_WIDTH, 18}, "WI-FI SETUP", 1,
                                 WatchyUi::Theme::foreground());
  WatchyUi::Widget::separator();
  if (state == WifiSetupState::CONNECTING) {
    AppVisual::drawStatusIcon({79, 39, 42, 42}, AppVisual::StatusIcon::RADIO,
                              true);
    WatchyUi::Canvas::centeredText({0, 92, 200, 18}, "CONNECTING", 2,
                                   WatchyUi::Theme::foreground());
    AppVisual::drawDataRow(146, "NETWORK", "Saved Wi-Fi", true);
  } else if (state == WifiSetupState::CONNECTED) {
    AppVisual::drawStatusIcon({79, 36, 42, 42}, AppVisual::StatusIcon::SUCCESS,
                              true);
    WatchyUi::Canvas::centeredText({0, 89, 200, 18}, "CONNECTED", 2,
                                   WatchyUi::Theme::foreground());
    AppVisual::drawDataRow(131, "SSID", ssid == nullptr ? "" : ssid, true);
    AppVisual::drawDataRow(156, "IP", ip == nullptr ? "" : ip);
    AppVisual::drawDataRow(178, "STATUS", "Saved");
  } else if (state == WifiSetupState::CANCELLED) {
    AppVisual::drawEmptyState({8, 40, 184, 116}, "SETUP CANCELLED",
                              "No network settings changed");
  } else if (state == WifiSetupState::TIMED_OUT) {
    AppVisual::drawWarningState({8, 40, 184, 116}, "WATCHY AP TIMED OUT",
                                "Restart setup to try again");
  } else {
    AppVisual::drawWarningState({8, 40, 184, 116}, "WI-FI SETUP FAILED",
                                "Could not start or join a network");
  }
  WatchyUi::Widget::footer(state == WifiSetupState::CONNECTING
                                ? "BACK CANCEL"
                                : "BACK EXIT");
  WatchyUi::Screen::present(APP_STATE);
}

} // namespace

bool connectWiFiImpl() {
  bool cancelled = false;
  bool connected = connectSavedWiFi(false, cancelled);
  if (!connected) turnOffWiFi();
  return connected;
}

bool Watchy::connectWiFi() { return connectWiFiImpl(); }

bool WatchySdk::connectWiFi() { return connectWiFiImpl(); }

void setupWifiImpl(Watchy *watchy) {
  bool returnToMenu = guiState == MAIN_MENU_STATE;
  guiState = APP_STATE;
  WatchyUi::Input::begin();

  drawWifiSetupState(WifiSetupState::CONNECTING);

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
    wifiManager.setAPCallback(WatchySdk::configModeCallback);
    wifiManager.startConfigPortal(WIFI_AP_SSID);

    if (!wifiManager.getConfigPortalActive()) {
      portalFailed = true;
    } else {
      uint32_t portalStartedAt = millis();
      uint32_t unattendedSince = millis();
      while (wifiManager.getConfigPortalActive() && !connected) {
        if (WatchyUi::Input::poll() == WatchyUi::Event::BACK) {
          cancelled = true;
          wifiManager.stopConfigPortal();
          break;
        }

        connected = wifiManager.process() || WiFi.status() == WL_CONNECTED;
        if (millis() - portalStartedAt >=
            WIFI_AP_ABSOLUTE_TIMEOUT * 1000UL) {
          timedOut = true;
          wifiManager.stopConfigPortal();
          break;
        } else if (WiFi.softAPgetStationNum() > 0) {
          unattendedSince = millis();
        } else if (millis() - unattendedSince >=
                   WIFI_AP_TIMEOUT * 1000UL) {
          timedOut = true;
          wifiManager.stopConfigPortal();
          break;
        }
        WatchyUi::deepSleepDelay(WIFI_PORTAL_SERVICE_INTERVAL_MS);
      }
      if (connected && wifiManager.getConfigPortalActive()) {
        wifiManager.stopConfigPortal();
      }
    }
  }

  if (connected) {
    rememberWiFiConnection();
    String address = WiFi.localIP().toString();
    drawWifiSetupState(WifiSetupState::CONNECTED, WiFi.SSID().c_str(),
                       address.c_str());
    weatherIntervalCounter = -1; // Reset to force weather to be read again
  } else if (cancelled) {
    drawWifiSetupState(WifiSetupState::CANCELLED);
  } else if (timedOut) {
    drawWifiSetupState(WifiSetupState::TIMED_OUT);
  } else if (portalFailed) {
    drawWifiSetupState(WifiSetupState::FAILED);
  } else {
    drawWifiSetupState(WifiSetupState::FAILED);
  }
  turnOffWiFi();
  if (cancelled && returnToMenu) {
    WatchyUi::Input::waitForRelease(WatchyUi::Event::BACK);
    if (watchy != nullptr) {
      watchy->showMenu(menuIndex, false);
    } else {
      WatchySdk::showMenu(menuIndex, false);
    }
  }
}

void Watchy::setupWifi() { setupWifiImpl(this); }

void WatchySdk::setupWifi() { setupWifiImpl(nullptr); }

void Watchy::_configModeCallback(WiFiManager *myWiFiManager) {
  WatchyUi::Screen::beginCanvas();
  Watchy::display.setFont(&FreeMonoBold9pt7b);
  Watchy::display.setTextColor(WatchyUi::Theme::foreground());
  Watchy::display.setCursor(0, 22);
  Watchy::display.println("WIFI SETUP");
  Watchy::display.println();
  Watchy::display.println("Join: Watchy AP");
  Watchy::display.println("Open browser:");
  Watchy::display.println("192.168.4.1");
  Watchy::display.println();
  Watchy::display.println("BACK to exit");
  WatchyUi::Screen::present(APP_STATE);
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
namespace WatchyDemo {

void renderWifiSetupPreview(uint8_t view) {
  switch (view) {
  case 0:
    drawWifiSetupState(WifiSetupState::CONNECTING);
    break;
  case 1:
    drawWifiSetupState(WifiSetupState::CONNECTED, "WATCHY_DEMO", "192.0.2.42");
    break;
  case 2:
    drawWifiSetupState(WifiSetupState::CANCELLED);
    break;
  case 3:
    drawWifiSetupState(WifiSetupState::TIMED_OUT);
    break;
  default:
    drawWifiSetupState(WifiSetupState::FAILED);
    break;
  }
}

} // namespace WatchyDemo
#endif
