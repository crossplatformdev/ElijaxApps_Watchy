#ifndef WATCHY_H
#define WATCHY_H

#include <Arduino.h>
#include <WiFiManager.h>
#include <HTTPClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Arduino_JSON.h>
#include <Wire.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include "DSEG7_Classic_Bold_53.h"
#include "Display.h"
#include "ThemeableDisplay.h"
#include "BLE.h"
#include "bma.h"
#include "config.h"
#include "esp_chip_info.h"
#include "TimezonesGMT.h"
// Global theme: dark (black background) vs light (white background).
// If DARKMODE is defined elsewhere (e.g. by a specific watchface), we
// respect that value for the initial boot, but runtime theme is controlled
// by the global variable gDarkMode.
extern bool gDarkMode;

#define THEME_BG (gDarkMode ? GxEPD_BLACK : GxEPD_WHITE)
#define THEME_FG (gDarkMode ? GxEPD_WHITE : GxEPD_BLACK)
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
  #define uS_TO_S_FACTOR 1000000ULL  //Conversion factor for micro seconds to seconds
  #define mS_TO_S_FACTOR 1000ULL     //Conversion factor for milliseconds to seconds
  #define ADC_VOLTAGE_DIVIDER ((360.0f+100.0f)/360.0f) //Voltage divider at battery ADC
  #define ACTIVE_LOW 0
#else
  #include "WatchyRTC.h"
  #define ACTIVE_LOW 1
#endif

typedef struct weatherData {
  int8_t temperature;
  int16_t weatherConditionCode;
  bool isMetric;
  String weatherDescription;
  bool external;
  tmElements_t sunrise;
  tmElements_t sunset;
} weatherData;

typedef struct watchySettings {
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
  long gmtOffset;
  //
  bool vibrateOClock;
} watchySettings;

class Watchy {
public:
  #ifdef ARDUINO_ESP32S3_DEV
   static Watchy32KRTC RTC;
  #else
   static WatchyRTC RTC;
  #endif
  static ThemeableGxEPD2_BW<WatchyDisplay, WatchyDisplay::HEIGHT> display;
  tmElements_t currentTime;
  watchySettings settings;

public:
  Watchy() = default;
  explicit Watchy(const watchySettings &s) : settings(s) {} // constructor
  void init(String datetime = "");
  void deepSleep();
  float getBatteryVoltage();
  uint8_t getBoardRevision();

  void vibMotor(uint8_t intervalMs = 100, uint8_t length = 20);
  void showMenu(byte menuIndex = 0);

  virtual void handleButtonPress();
  void showFastMenu(byte menuIndex);
  void showAbout();
  void showBuzz();
  void showAccelerometer();
  void showUpdateFW();
  void showSyncNTP();
  bool syncNTP();
  bool syncNTP(long int gmt);
  bool syncNTP(long int gmt, String ntpServer);
  void showInvertColors();
  void showWatchfaceSelector();
  // New apps
  void showPing();
  void showTraceroute();
  void showDig();
  void showWhois();
  void showPortScanner();
  void showPostman();
  void showNewsReader();
  void showGoogleSearch();
  void showTextBrowser(const String &url);
  void showTextBrowserPostForm(const String &url,
                               const String &formBody,
                               const String &origin = String(),
                               const String &referer = String(),
                               const String &acceptLanguage = String());
  void showTextBrowserHome();
  void showRadio();
  void showMorseGame();
  void showMoonPhase();
  void showSunRise();
  void showMoonRise();
  void showChronometer();


  // -------------------------------------------------------------------------
  // UI control callbacks (used by UiSDK controls row)
  // -------------------------------------------------------------------------
  // All apps/OS screens should use ONLY these four callbacks in
  // UIControlsRowLayout.callback.
  void backPressed();
  void menuPressed();
  void upPressed();
  void downPressed();

  // Apps/templates can temporarily install per-screen handlers. This avoids
  // defining per-app Watchy::backPressed/menuPressed/... implementations.
  using ButtonHandler = void (*)(Watchy *watchy);
  void setButtonHandlers(ButtonHandler back,
                         ButtonHandler up,
                         ButtonHandler menu,
                         ButtonHandler down);
  void clearButtonHandlers();

  void showAlarm();
  void showTimer();
  void showSetTime();
  void setupWifi();
  bool connectWiFi();
  weatherData getWeatherData();
  void updateFWBegin();

  // Menu helpers (grouped menu/submenu navigation)
  uint8_t activeMenuLength() const;
  bool isInSubMenu() const;
  void enterSubMenu(uint8_t categoryIndex);
  void returnToTopMenu();
  void launchMenuAction(uint8_t categoryIndex, uint8_t itemIndex);

  void showWatchFace(bool partialRefresh);
  virtual void drawWatchFace(); // override this method for different watch
                                // faces

private:
  void _bmaConfig();
  static void _configModeCallback(WiFiManager *myWiFiManager);
  static uint16_t _readRegister(uint8_t address, uint8_t reg, uint8_t *data,
                                uint16_t len);
  static uint16_t _writeRegister(uint8_t address, uint8_t reg, uint8_t *data,
                                 uint16_t len);
  void checkAlarmTrigger();
  weatherData _getWeatherData(String cityID, String lat, String lon, String units, String lang,
                             String url, String apiKey, uint8_t updateInterval);                                 
};

extern RTC_DATA_ATTR int guiState;
extern RTC_DATA_ATTR int menuIndex;
extern RTC_DATA_ATTR BMA423 sensor;
extern RTC_DATA_ATTR bool WIFI_CONFIGURED;
extern RTC_DATA_ATTR bool BLE_CONFIGURED;
extern RTC_DATA_ATTR bool USB_PLUGGED_IN;
extern RTC_DATA_ATTR weatherData currentWeather;
extern RTC_DATA_ATTR int weatherIntervalCounter;
extern RTC_DATA_ATTR long gmtOffset;
extern RTC_DATA_ATTR bool alreadyInMenu;
extern RTC_DATA_ATTR tmElements_t bootTime;
extern RTC_DATA_ATTR uint32_t lastIPAddress;
extern RTC_DATA_ATTR char lastSSID[30];
extern RTC_DATA_ATTR char lastPassword[64];
extern RTC_DATA_ATTR uint8_t currentWatchfaceId;
extern RTC_DATA_ATTR bool gDarkMode;      // runtime theme (true=dark background)
extern RTC_DATA_ATTR uint8_t menuLevel;     // 0 = top-level categories, 1 = submenu
extern RTC_DATA_ATTR uint8_t menuCategory;  // active top-level category when in submenu
extern RTC_DATA_ATTR uint32_t lastDeepSleepTime;
#endif