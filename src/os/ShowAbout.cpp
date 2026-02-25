#include <Watchy.h>
#include "sdk/WatchyUi.h"

extern RTC_DATA_ATTR bool WIFI_CONFIGURED;
extern RTC_DATA_ATTR long gmtOffset;
extern RTC_DATA_ATTR char lastSSID[33];
extern RTC_DATA_ATTR uint32_t lastIPAddress;

void Watchy::showAbout() {
  display.setFullWindow();
  display.fillScreen(DARKMODE ? GxEPD_BLACK : GxEPD_WHITE);
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);;
  display.setCursor(0, 20);

  display.print("LibVer: ");
  display.println(WATCHY_LIB_VER);

  display.print("Rev: v");
  display.println(getBoardRevision());

  display.print("Batt: ");
  float voltage = getBatteryVoltage();
  display.print(voltage);
  display.println("V");

  #ifndef ARDUINO_ESP32S3_DEV
  display.print("Uptime: ");
  RTC.read(currentTime);
  time_t b = makeTime(bootTime);
  time_t c = makeTime(currentTime);
  int totalSeconds = c-b;
  //int seconds = (totalSeconds % 60);
  int minutes = (totalSeconds % 3600) / 60;
  int hours = (totalSeconds % 86400) / 3600;
  int days = (totalSeconds % (86400 * 30)) / 86400; 
  display.print(days);
  display.print("d");
  display.print(hours);
  display.print("h");
  display.print(minutes);
  display.println("m");  
  #endif
  
  if(WIFI_CONFIGURED){
    display.print("SSID: ");
    display.println(lastSSID);
    display.print("IP: ");
    display.println(IPAddress(lastIPAddress).toString());
  }else{
    display.println("WiFi Not Connected");
  }
  display.display(true); // partial refresh

  guiState = APP_STATE;
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
namespace WatchyDemo {

void renderAboutPreview() {
  Watchy::display.setFullWindow();
  Watchy::display.fillScreen(WatchyUi::Theme::background());
  Watchy::display.setFont(&FreeMonoBold9pt7b);
  Watchy::display.setTextColor(WatchyUi::Theme::foreground());
  Watchy::display.setCursor(0, 20);
  Watchy::display.print("LibVer: ");
  Watchy::display.println(WATCHY_LIB_VER);
  Watchy::display.println("Rev: v3");
  Watchy::display.println("Batt: 3.82V");
  Watchy::display.println("SSID: WATCHY_DEMO");
  Watchy::display.println("IP: 192.0.2.42");
  Watchy::display.display(true);
}

} // namespace WatchyDemo
#endif