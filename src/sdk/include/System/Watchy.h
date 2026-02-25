#ifndef WATCHY_H
#define WATCHY_H

#include <Arduino.h>
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#endif
#include <GxEPD2_BW.h>
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
#include <Fonts/FreeMonoBold9pt7b.h>
#include "Display.h"
#include "bma.h"
#include "MenuModel.h"
#include "config.h"
#ifdef ARDUINO_ESP32S3_DEV
#include "Watchy32KRTC.h"
#else
#include "WatchyRTC.h"
#endif

class WiFiManager;

#ifdef ARDUINO_ESP32S3_DEV
#define ACTIVE_LOW 0
#else
#define ACTIVE_LOW 1
#endif

extern RTC_DATA_ATTR bool DARKMODE;
extern const GFXfont DSEG7_Classic_Bold_53;

typedef struct weatherData
{
  int8_t temperature;
  int16_t weatherConditionCode;
  bool isMetric;
  bool external;
  tmElements_t sunrise;
  tmElements_t sunset;
} weatherData;

typedef struct watchySettings
{
  // Weather Settings
  String cityID;
  String lat;
  String lon;
  String weatherAPIKey;
  String weatherURL;
  String weatherUnit;
  String weatherLang;
  int8_t weatherUpdateInterval;
  // NTP Settings
  String ntpServer;
  int gmtOffset;
  //
  bool vibrateOClock;
} watchySettings;

class Watchy
{
public:
#ifdef ARDUINO_ESP32S3_DEV
  static Watchy32KRTC RTC;
#else
  static WatchyRTC RTC;
#endif
  static GxEPD2_BW<WatchyDisplay, WatchyDisplay::HEIGHT> display;
  tmElements_t currentTime;
  watchySettings settings;
  
  explicit Watchy(const watchySettings &s) : settings(s) {} // constructor
  static void setLowPowerCpuFrequency();
  static void setRadioCpuFrequency();
  static void ensureDisplayInitialized();
  static uint32_t uptimeSeconds();
  void init(String datetime = "");
  void deepSleep();
  static float getBatteryVoltage();
  static uint8_t getBoardRevision();
  static void vibMotor(uint16_t intervalMs = 100, uint8_t length = 20);

  virtual void handleButtonPress();
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
  weatherData getWeatherData();
  weatherData getCachedWeatherData() const;

  void showWatchFace(bool updateData = true);
  virtual void drawWatchFace(); // override this method for different watch
                                // faces
  virtual bool updateWatchFaceData();
  virtual bool refreshWatchFaceHeartRate();

private:
  void selectMenuEntry();
  void leaveMenuLevel();
  void moveMenuSelection(int direction, bool fastRefresh);
  void _bmaConfig();
  static void _configModeCallback(WiFiManager *myWiFiManager);
  static uint16_t _readRegister(uint8_t address, uint8_t reg, uint8_t *data,
                                uint16_t len);
  static uint16_t _writeRegister(uint8_t address, uint8_t reg, uint8_t *data,
                                 uint16_t len);
  weatherData _getWeatherData(String cityID, String lat, String lon, String units, String lang,
                              String url, String apiKey, uint8_t updateInterval);
};

extern RTC_DATA_ATTR int guiState;
extern RTC_DATA_ATTR int menuIndex;
extern RTC_DATA_ATTR uint8_t menuLevel;
extern RTC_DATA_ATTR uint8_t menuCategory;
extern RTC_DATA_ATTR int categoryMenuIndex;
extern RTC_DATA_ATTR uint8_t submenuMenuIndices[MENU_CATEGORY_COUNT];
extern RTC_DATA_ATTR uint8_t menuLayoutVersion;
extern RTC_DATA_ATTR BMA423 sensor;
extern RTC_DATA_ATTR bool WIFI_CONFIGURED;
extern RTC_DATA_ATTR bool BLE_CONFIGURED;
extern RTC_DATA_ATTR weatherData currentWeather;
extern RTC_DATA_ATTR int weatherIntervalCounter;
extern RTC_DATA_ATTR long gmtOffset;
extern RTC_DATA_ATTR bool alreadyInMenu;
extern RTC_DATA_ATTR bool USB_PLUGGED_IN;
extern RTC_DATA_ATTR tmElements_t bootTime;
extern RTC_DATA_ATTR uint32_t lastIPAddress;
extern RTC_DATA_ATTR char lastSSID[33];

#endif