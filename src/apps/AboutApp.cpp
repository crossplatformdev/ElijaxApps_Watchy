#include "../watchy/Watchy.h"
#include "../sdk/UiSDK.h"
#include "../sdk/UiTemplates.h"

void Watchy::showAbout() {
  // Build the "About" screen using the UI SDK
  const uint16_t color = UiSDK::getWatchfaceFg(BASE_POLARITY);

  // First, assemble dynamic strings
  String libVer  = String("LibVer: ") + WATCHY_LIB_VER;
  String rev     = String("Rev: v") + String(getBoardRevision());
  float voltage  = getBatteryVoltage();
  String batt    = String("Batt: ") + String(voltage, 2) + "V";

  String uptimeStr;
  RTC.read(currentTime);
  {
    time_t b = makeTime(bootTime);
    time_t c = makeTime(currentTime);
    int totalSeconds = c - b;
    int minutes = (totalSeconds % 3600) / 60;
    int hours   = (totalSeconds % 86400) / 3600;
    int days    = (totalSeconds % (86400 * 30)) / 86400;
    uptimeStr   = String("Uptime: ") + String(days) + "d" + String(hours) + "h" + String(minutes) + "m";
  }

  String wifiLine1;
  String wifiLine2;
  if (WIFI_CONFIGURED) {
    wifiLine1 = String("SSID: ") + String(lastSSID);
    wifiLine2 = String("IP: ") + IPAddress(lastIPAddress).toString();
  } else {
    wifiLine1 = String("WiFi Not Connected");
  }

  // Layout texts vertically
  UITextSpec textSpecs[6];
  uint8_t idx = 0;
  // Initialize each text spec with background fill so colours don't swap
  textSpecs[idx].x               = 0;
  textSpecs[idx].y               = 20;
  textSpecs[idx].w               = 0;
  textSpecs[idx].h               = 0;
  textSpecs[idx].font            = &FreeMonoBold9pt7b;
  textSpecs[idx].fillBackground  = false;
  textSpecs[idx].text            = libVer;
  textSpecs[idx].invert          = false;
  idx++;

  textSpecs[idx].x               = 0;
  textSpecs[idx].y               = 40;
  textSpecs[idx].w               = 0;
  textSpecs[idx].h               = 0;
  textSpecs[idx].font            = &FreeMonoBold9pt7b;
  textSpecs[idx].fillBackground  = false;
  textSpecs[idx].text            = rev;
  textSpecs[idx].invert          = false;
  idx++;

  textSpecs[idx].x               = 0;
  textSpecs[idx].y               = 60;
  textSpecs[idx].w               = 0;
  textSpecs[idx].h               = 0;
  textSpecs[idx].font            = &FreeMonoBold9pt7b;
  textSpecs[idx].fillBackground  = false;
  textSpecs[idx].text            = batt;
  textSpecs[idx].invert          = false;
  idx++;
  textSpecs[idx].x               = 0;
  textSpecs[idx].y               = 80;
  textSpecs[idx].w               = 0;
  textSpecs[idx].h               = 0;
  textSpecs[idx].font            = &FreeMonoBold9pt7b;
  textSpecs[idx].fillBackground  = false;
  textSpecs[idx].text            = uptimeStr;
  textSpecs[idx].invert          = false;
  idx++;
  textSpecs[idx].x               = 0;
  textSpecs[idx].y               = 100;
  textSpecs[idx].w               = 0;
  textSpecs[idx].h               = 0;
  textSpecs[idx].font            = &FreeMonoBold9pt7b;
  textSpecs[idx].fillBackground  = false;
  textSpecs[idx].text            = wifiLine1;
  textSpecs[idx].invert          = false;
  idx++;
  if (wifiLine2.length() > 0) {
    textSpecs[idx].x               = 0;
    textSpecs[idx].y               = 120;
    textSpecs[idx].w               = 0;
    textSpecs[idx].h               = 0;
    textSpecs[idx].font            = &FreeMonoBold9pt7b;
    textSpecs[idx].fillBackground  = false;
    textSpecs[idx].text            = wifiLine2;    
    textSpecs[idx].invert          = false;
    idx++;
  }

  UIAppSpec app{};
  app.texts        = textSpecs;
  app.textCount    = idx;
  app.images       = nullptr;
  app.imageCount   = 0;
  app.menus        = nullptr;
  app.menuCount    = 0;
  app.buttons      = nullptr;
  app.buttonCount  = 0;

  UiTemplates::renderBarePage(*this, app);

  guiState = APP_STATE;
}
