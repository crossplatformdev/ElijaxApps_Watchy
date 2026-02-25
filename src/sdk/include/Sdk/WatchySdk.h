#ifndef WATCHY_SDK_H
#define WATCHY_SDK_H

#include <Arduino.h>
#include <WiFiManager.h>
#include <HTTPClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Arduino_JSON.h>
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#endif
#include <GxEPD2_BW.h>
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
#include <Wire.h>
#include "Display.h"
#include "BLE.h"
#include "bma.h"
#include "config.h"
#include "esp_chip_info.h"
#ifdef ARDUINO_ESP32S3_DEV
  #include "Watchy32KRTC.h"
  #include "soc/rtc.h"
  #include "soc/rtc_io_reg.h"
  #include "soc/sens_reg.h"
  #include "esp_sleep.h"
  #include "rom/rtc.h"
  #include "soc/soc.h"
  #include "soc/rtc_cntl_reg.h"
  #include "time.h"
  #include "esp_sntp.h"
  #include "hal/rtc_io_types.h"
  #include "driver/rtc_io.h"
#else
  #include "WatchyRTC.h"
#endif
#include "settings.h"
#ifdef ARDUINO_ESP32S3_DEV
  #define ACTIVE_LOW 0
#else
  #define ACTIVE_LOW 1
#endif

extern RTC_DATA_ATTR bool DARKMODE;
extern const GFXfont DSEG7_Classic_Bold_53;

// WatchySdk is the entire public surface of the firmware: device state lives
// under WatchySdk::Device, behaviour is exposed as plain WatchySdk functions.
namespace WatchySdk {

namespace Device {
#ifdef ARDUINO_ESP32S3_DEV
  extern Watchy32KRTC RTC;
#else
  extern WatchyRTC RTC;
#endif
  extern GxEPD2_BW<WatchyDisplay, WatchyDisplay::HEIGHT> display;
  extern RTC_DATA_ATTR tmElements_t currentTime;
  extern RTC_DATA_ATTR watchySettings settings;
} // namespace Device

// Bring device state into WatchySdk scope so existing app code that refers
// to `display`, `currentTime`, and `settings` unqualified keeps working.
using Device::RTC;
using Device::display;
using Device::currentTime;
using Device::settings;

void setLowPowerCpuFrequency();
void setRadioCpuFrequency();
void ensureDisplayInitialized();
uint32_t uptimeSeconds();
void init(String datetime = "");
void deepSleep();
float getBatteryVoltage();
uint8_t getBoardRevision();
void vibMotor(uint16_t intervalMs = 100, uint8_t length = 20);

void handleButtonPress();
void showMenu(byte menuIndex, bool partialRefresh);
void showFastMenu(byte menuIndex, byte previousIndex);
void showAbout();
void showBuzz();
void showAccelerometer();
void showSyncNTP();
void showSunRise();
void showMoonRise();
void showMoonPhase();
void showTides();
void showHeartRate();
void showMorseGuessLetter();
void showMorseGuessCode();
void showPong();
void showSnake();
void showOthello();
void showWatchfaceSelector();
void showThemeColours();
void showBrowser();
void showRssFeed();
void showPing();
void showTraceroute();
void showPortScanner();
void showDnsQuery();
void showReverseDnsQuery();
void showDuckDuckGo();
void showWifiSurvey();
void showClockTool(uint8_t tool);
void showTimerTool(uint8_t tool);
void showSensorTool(uint8_t tool);
void showBluetoothTool(uint8_t tool);
void showHealthcareTool(uint8_t tool);
void showSafetyTool(uint8_t tool);
void showHealthReminder(uint8_t tool);
void showHealthSupportTool(uint8_t tool);
void showUtilityTool(uint8_t tool);
void showMiniGame(uint8_t game);
bool syncNTP();
bool syncNTP(long gmt);
bool syncNTP(long gmt, String ntpServer);
void setTime();
void setupWifi();
bool connectWiFi();
void configModeCallback(WiFiManager *myWiFiManager);
weatherData getWeatherData();
weatherData getCachedWeatherData();

void showWatchFace(bool updateData = true);
void drawWatchFace();
bool updateWatchFaceData();
bool refreshWatchFaceHeartRate();
#ifdef WATCHY_DETERMINISTIC_GALLERY
void drawGalleryWatchface(uint8_t watchfaceId, const tmElements_t &fixedTime);
#endif

} // namespace WatchySdk

extern RTC_DATA_ATTR int guiState;
extern RTC_DATA_ATTR int menuIndex;
extern RTC_DATA_ATTR BMA423 sensor;
extern RTC_DATA_ATTR bool WIFI_CONFIGURED;
extern RTC_DATA_ATTR bool BLE_CONFIGURED;
extern RTC_DATA_ATTR bool USB_PLUGGED_IN;

#endif
