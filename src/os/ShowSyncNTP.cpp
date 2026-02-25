#include <Watchy.h>
#include "sdk/WatchyUi.h"

extern RTC_DATA_ATTR long gmtOffset;

void Watchy::showSyncNTP() {
  display.setFullWindow();
  display.fillScreen(DARKMODE ? GxEPD_BLACK : GxEPD_WHITE);
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);;
  display.setCursor(0, 30);
  display.println("Syncing NTP... ");
  display.print("GMT offset: ");
  display.println(gmtOffset);
  display.display(true); // partial refresh
  if (connectWiFi()) {
    if (syncNTP()) {
      display.println("NTP Sync Success\n");
      display.println("Current Time Is:");

      RTC.read(currentTime);

      display.print(tmYearToCalendar(currentTime.Year));
      display.print("/");
      display.print(currentTime.Month);
      display.print("/");
      display.print(currentTime.Day);
      display.print(" - ");

      if (currentTime.Hour < 10) {
        display.print("0");
      }
      display.print(currentTime.Hour);
      display.print(":");
      if (currentTime.Minute < 10) {
        display.print("0");
      }
      display.println(currentTime.Minute);
    } else {
      display.println("NTP Sync Failed");
    }
  } else {
    display.println("WiFi Not Configured");
  }
  WiFi.disconnect(true, false);
  btStop();
  setLowPowerCpuFrequency();
  display.display(true); // partial refresh
  delay(3000);
  showMenu(menuIndex, false);
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
namespace WatchyDemo {

void renderSyncNtpPreview() {
  Watchy::display.setFullWindow();
  Watchy::display.fillScreen(WatchyUi::Theme::background());
  Watchy::display.setFont(&FreeMonoBold9pt7b);
  Watchy::display.setTextColor(WatchyUi::Theme::foreground());
  Watchy::display.setCursor(0, 30);
  Watchy::display.println("NTP Sync Success");
  Watchy::display.println();
  Watchy::display.println("Current Time Is:");
  Watchy::display.println("2026/8/23 - 10:34");
  Watchy::display.setCursor(0, 165);
  Watchy::display.println("GMT offset: +02:00");
  Watchy::display.display(true);
}

} // namespace WatchyDemo
#endif