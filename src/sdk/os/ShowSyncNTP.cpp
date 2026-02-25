#include "WatchyUi.h"
#include "AppDisplay.h"
#include "WatchyPowerDiagnostics.h"
#include "Watchy.h"

extern RTC_DATA_ATTR long gmtOffset;

namespace {

void formatGmtOffset(char output[16], long offset) {
  long absoluteOffset = offset < 0 ? -offset : offset;
  snprintf(output, 16, "%c%02ld:%02ld", offset < 0 ? '-' : '+',
           absoluteOffset / SECS_PER_HOUR,
           absoluteOffset % SECS_PER_HOUR / SECS_PER_MIN);
}

void drawNtpStatus(bool syncing, bool success, const tmElements_t *time,
                   const char *detail) {
  WatchyUi::Screen::beginCanvas();
  Watchy::display.setFont();
  Watchy::display.setTextSize(1);
  WatchyUi::Canvas::centeredText({0, 1, DISPLAY_WIDTH, 18}, "NTP SYNC", 1,
                                 WatchyUi::Theme::foreground());
  WatchyUi::Widget::separator();
  char offset[16];
  formatGmtOffset(offset, gmtOffset);
  if (syncing) {
    AppVisual::drawStatusIcon({79, 39, 42, 42}, AppVisual::StatusIcon::RADIO,
                              true);
    WatchyUi::Canvas::centeredText({0, 92, 200, 18}, "SYNCHRONIZING", 2,
                                   WatchyUi::Theme::foreground());
    AppVisual::drawDataRow(146, "GMT OFFSET", offset, true);
  } else if (success && time != nullptr) {
    char value[6];
    char date[16];
    WatchyUi::Selector::formatTime(value, time->Hour, time->Minute);
    snprintf(date, sizeof(date), "%04u-%02u-%02u",
             tmYearToCalendar(time->Year), time->Month, time->Day);
    AppVisual::drawMetric({12, 36, 176, 72}, "TIME SYNCHRONIZED", value);
    AppVisual::drawDataRow(133, "DATE", date, true);
    AppVisual::drawDataRow(155, "GMT OFFSET", offset);
    AppVisual::drawDataRow(177, "SERVER", "NTP OK");
  } else {
    AppVisual::drawWarningState({8, 40, 184, 116}, "SYNC FAILED", detail);
  }
  WatchyUi::Widget::footer(syncing ? "BACK CANCEL" : "BACK EXIT");
  WatchyUi::Screen::present(APP_STATE);
}

} // namespace

void showSyncNtpImpl(Watchy *watchy) {
  drawNtpStatus(true, false, nullptr, nullptr);
  bool connected = watchy != nullptr ? watchy->connectWiFi()
                                     : WatchySdk::connectWiFi();
  if (connected) {
    bool synced = watchy != nullptr ? watchy->syncNTP()
                                    : WatchySdk::syncNTP();
    if (synced) {
      tmElements_t currentTime;
      if (watchy != nullptr) {
        watchy->RTC.read(watchy->currentTime);
        currentTime = watchy->currentTime;
      } else {
        WatchySdk::RTC.read(WatchySdk::currentTime);
        currentTime = WatchySdk::currentTime;
      }
      drawNtpStatus(false, true, &currentTime, nullptr);
    } else {
      drawNtpStatus(false, false, nullptr, "The NTP server did not respond");
    }
  } else {
    drawNtpStatus(false, false, nullptr, "Wi-Fi is unavailable");
  }
  WiFi.disconnect(true, false);
  btStop();
  WatchyDiagnostics::endWifiSession();
  Watchy::setLowPowerCpuFrequency();
  WatchyUi::deepSleepDelay(3000);
  if (watchy != nullptr) {
    watchy->showMenu(menuIndex, false);
  } else {
    WatchySdk::showMenu(menuIndex, false);
  }
}

void Watchy::showSyncNTP() { showSyncNtpImpl(this); }

void WatchySdk::showSyncNTP() { showSyncNtpImpl(nullptr); }

bool Watchy::syncNTP() { // NTP sync - call after connecting to WiFi and
                         // remember to turn it back off
  return syncNTP(gmtOffset,
                 settings.ntpServer.c_str());
}

bool Watchy::syncNTP(long gmt) {
  return syncNTP(gmt, settings.ntpServer.c_str());
}

bool Watchy::syncNTP(long gmt, String ntpServer) {
  // NTP sync - call after connecting to
  // WiFi and remember to turn it back off
  WiFiUDP ntpUDP;
  NTPClient timeClient(ntpUDP, ntpServer.c_str(), gmt);
  timeClient.begin();
  if (!timeClient.forceUpdate()) {
    return false; // NTP sync failed
  }
  tmElements_t tm;
  breakTime((time_t)timeClient.getEpochTime(), tm);
  RTC.set(tm);
  return true;
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
namespace WatchyDemo {

void renderSyncNtpPreview(uint8_t view) {
  long previousOffset = gmtOffset;
  gmtOffset = 2 * SECS_PER_HOUR;
  if (view == 0) {
    drawNtpStatus(true, false, nullptr, nullptr);
  } else if (view == 1) {
    tmElements_t time{};
    time.Year = CalendarYrToTm(2026);
    time.Month = 8;
    time.Day = 23;
    time.Hour = 10;
    time.Minute = 34;
    drawNtpStatus(false, true, &time, nullptr);
  } else {
    drawNtpStatus(false, false, nullptr, "Wi-Fi is unavailable");
  }
  gmtOffset = previousOffset;
}

} // namespace WatchyDemo
#endif
