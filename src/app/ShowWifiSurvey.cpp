#include "NetworkAppCommon.h"
#include "AppDisplay.h"
#include <esp_wifi.h>
#include "WatchyPowerDiagnostics.h"


#include "WatchyUi.h"
#include "Watchy.h"

namespace {

constexpr uint32_t wifiScanDeadlineMs = 8000;
constexpr uint32_t wifiScanPollMs = 100;

uint8_t signalStrength(int rssi) {
  if (rssi >= -55) return 4;
  if (rssi >= -65) return 3;
  if (rssi >= -78) return 2;
  if (rssi >= -90) return 1;
  return 0;
}

String clippedName(const String &name, uint8_t maximum = 15) {
  return name.length() <= maximum ? name : name.substring(0, maximum - 3) + "...";
}

void drawWifiRow(int16_t y, const String &ssid, int rssi, int channel,
                 bool secured, bool emphasized) {
  if (emphasized) {
    WatchyUi::GrayPaint::fillRoundRect(
        {8, static_cast<int16_t>(y - 12), 184, 17}, 3,
        WatchyUi::Theme::tone(WatchyUi::ToneRole::Surface));
  }
  const int16_t rowCenterY = static_cast<int16_t>(y - 12 + 17 / 2);
    Watchy::display.setFont();
    Watchy::display.setTextSize(1);
    Watchy::display.setTextColor(WatchyUi::Theme::foreground());
  String name = clippedName(ssid);
  String detail = String(channel) + (secured ? "S" : "O");
    Watchy::display.setCursor(
      12, AppVisual::centeredCursorY(rowCenterY, name.c_str()));
    Watchy::display.print(name);
    Watchy::display.setCursor(
      106, AppVisual::centeredCursorY(rowCenterY, detail.c_str()));
    Watchy::display.print(detail);
  AppVisual::drawSignalBars({143, static_cast<int16_t>(y - 12), 42, 15},
                            signalStrength(rssi));
}

void drawWifiSurveyResult(int networkCount) {
  beginAppDisplay("WIFI SURVEY");
  if (networkCount == WIFI_SCAN_FAILED) {
    AppVisual::drawWarningState({8, 40, 184, 116}, "SCAN FAILED",
                                "Wi-Fi scan timed out or could not start");
  } else if (networkCount == 0) {
    AppVisual::drawEmptyState({8, 40, 184, 116}, "NO NETWORKS",
                              "No access points were detected");
  } else {
    char count[8];
    snprintf(count, sizeof(count), "%d", networkCount);
    AppVisual::drawMetric({12, 32, 176, 58}, "NETWORKS FOUND", count);
    int visibleCount = min(networkCount, 4);
    for (int index = 0; index < visibleCount; index++) {
      drawWifiRow(116 + index * 20, WiFi.SSID(index), WiFi.RSSI(index),
                  WiFi.channel(index),
                  WiFi.encryptionType(index) != WIFI_AUTH_OPEN, index == 0);
    }
  }
  WatchyUi::Widget::footer("SIGNAL SORTED  BACK EXIT");
  finishAppDisplay();
}

} // namespace

void showWifiSurveyImpl(Watchy *watchy) {
  NetworkApps::prepareButtons();
  WatchyDiagnostics::beginWifiSession();
  Watchy::setRadioCpuFrequency();
  WiFi.mode(WIFI_STA);
  NetworkApps::showStatus("WIFI SURVEY", "Passive channel scan...");
  int networkCount = WiFi.scanNetworks(true, true, true, 300);
  bool cancelled = false;
  uint32_t scanStartedAt = millis();
  while (networkCount == WIFI_SCAN_RUNNING &&
         millis() - scanStartedAt < wifiScanDeadlineMs) {
    if (WatchyUi::Input::wait(wifiScanPollMs) ==
        WatchyUi::Event::BACK) {
      cancelled = true;
      break;
    }
    networkCount = WiFi.scanComplete();
  }
  if (cancelled || networkCount == WIFI_SCAN_RUNNING) {
    esp_wifi_scan_stop();
    WiFi.scanDelete();
    NetworkApps::disconnect();
    if (cancelled) {
      if (watchy != nullptr) {
        watchy->showMenu(menuIndex, false);
      } else {
        WatchySdk::showMenu(menuIndex, false);
      }
      return;
    }
    networkCount = WIFI_SCAN_FAILED;
  }
  drawWifiSurveyResult(networkCount);
  WiFi.scanDelete();
  NetworkApps::disconnect();
  NetworkApps::prepareButtons();
  while (NetworkApps::waitForButton() != NetworkApps::BUTTON_BACK) {}
  if (watchy != nullptr) {
    watchy->showMenu(menuIndex, false);
  } else {
    WatchySdk::showMenu(menuIndex, false);
  }
}

void Watchy::showWifiSurvey() { showWifiSurveyImpl(this); }

void WatchySdk::showWifiSurvey() { showWifiSurveyImpl(nullptr); }
