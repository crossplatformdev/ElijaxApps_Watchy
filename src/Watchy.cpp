#include "WatchyUi.h"
#include "WatchyPowerDiagnostics.h"
#include "SensorManager.h"

#include "Watchy.h"
#include <type_traits>

namespace {

constexpr uint32_t LOW_POWER_CPU_FREQUENCY_MHZ = 40;
constexpr uint32_t RADIO_CPU_FREQUENCY_MHZ = 80;
constexpr uint8_t MENU_LAYOUT_VERSION = 1;

void sensorDelay(uint32_t durationMs) {
  WatchyUi::deepSleepDelay(durationMs);
}

} // namespace

#ifdef ARDUINO_ESP32S3_DEV
  Watchy32KRTC Watchy::RTC;
#else
  WatchyRTC Watchy::RTC;
#endif
GxEPD2_BW<WatchyDisplay, WatchyDisplay::HEIGHT> Watchy::display(
    WatchyDisplay{});

RTC_DATA_ATTR int guiState = WATCHFACE_STATE;
RTC_DATA_ATTR int menuIndex = 0;
RTC_DATA_ATTR uint8_t menuLevel = MENU_LEVEL_CATEGORIES;
RTC_DATA_ATTR uint8_t menuCategory = 0;
RTC_DATA_ATTR int categoryMenuIndex = 0;
RTC_DATA_ATTR uint8_t submenuMenuIndices[MENU_CATEGORY_COUNT] = {};
RTC_DATA_ATTR uint8_t menuLayoutVersion = 0;
RTC_DATA_ATTR BMA423 sensor;
RTC_DATA_ATTR bool WIFI_CONFIGURED = false;
RTC_DATA_ATTR bool BLE_CONFIGURED = false;
RTC_DATA_ATTR weatherData currentWeather{};
static_assert(std::is_trivially_copyable<weatherData>::value,
              "RTC weather data must not own dynamic memory");
RTC_DATA_ATTR int weatherIntervalCounter = -1;
RTC_DATA_ATTR long gmtOffset = 0;
RTC_DATA_ATTR bool alreadyInMenu         = true;
RTC_DATA_ATTR bool DARKMODE = true;
RTC_DATA_ATTR bool USB_PLUGGED_IN = false;
RTC_DATA_ATTR tmElements_t bootTime{};
RTC_DATA_ATTR uint32_t lastIPAddress = 0;
RTC_DATA_ATTR char lastSSID[33] = {};

void Watchy::setLowPowerCpuFrequency() {
  setCpuFrequencyMhz(LOW_POWER_CPU_FREQUENCY_MHZ);
}

void Watchy::setRadioCpuFrequency() {
  setCpuFrequencyMhz(RADIO_CPU_FREQUENCY_MHZ);
}

void Watchy::ensureDisplayInitialized() {
  display.epd2.initWatchy();
}

uint32_t Watchy::uptimeSeconds() {
  return WatchySdk::uptimeSeconds();
}

void Watchy::init(String datetime) {
  WatchySdk::settings = settings;
  WatchySdk::init(datetime);
}
void Watchy::deepSleep() {
  WatchySdk::deepSleep();
}

void Watchy::handleButtonPress() {
  WatchySdk::handleButtonPress();
}

void Watchy::selectMenuEntry() {
  if (menuLevel == MENU_LEVEL_CATEGORIES) {
    categoryMenuIndex = menuIndex;
    menuCategory = menuIndex;
    menuLevel = MENU_LEVEL_APPLICATIONS;
    menuIndex = submenuMenuIndices[menuCategory];
    showMenu(menuIndex, false);
    return;
  }

  submenuMenuIndices[menuCategory] = menuIndex;
  MenuAction action = getMenuAction(menuCategory, menuIndex);
  switch (action) {
  case MENU_ACTION_ABOUT: showAbout(); break;
  case MENU_ACTION_SET_TIME: setTime(); break;
  case MENU_ACTION_SETUP_WIFI: setupWifi(); break;
  case MENU_ACTION_WATCH_FACES: showWatchfaceSelector(); break;
  case MENU_ACTION_THEME_COLOURS: showThemeColours(); break;
  case MENU_ACTION_VIBRATE: showBuzz(); break;
  case MENU_ACTION_ACCELEROMETER: showAccelerometer(); break;
  case MENU_ACTION_SYNC_NTP: showSyncNTP(); break;
  case MENU_ACTION_SUN_RISE: showSunRise(); break;
  case MENU_ACTION_MOON_RISE: showMoonRise(); break;
  case MENU_ACTION_MOON_PHASE: showMoonPhase(); break;
  case MENU_ACTION_TIDES: showTides(); break;
  case MENU_ACTION_HEART_RATE: showHeartRate(); break;
  case MENU_ACTION_MORSE_LETTER: showMorseGuessLetter(); break;
  case MENU_ACTION_MORSE_CODE: showMorseGuessCode(); break;
  case MENU_ACTION_PONG: showPong(); break;
  case MENU_ACTION_SNAKE: showSnake(); break;
  case MENU_ACTION_OTHELLO: showOthello(); break;
  case MENU_ACTION_BROWSER: showBrowser(); break;
  case MENU_ACTION_RSS_FEED: showRssFeed(); break;
  case MENU_ACTION_PING: showPing(); break;
  case MENU_ACTION_TRACEROUTE: showTraceroute(); break;
  case MENU_ACTION_PORT_SCANNER: showPortScanner(); break;
  case MENU_ACTION_DNS_QUERY: showDnsQuery(); break;
  case MENU_ACTION_REVERSE_DNS: showReverseDnsQuery(); break;
  case MENU_ACTION_DUCKDUCKGO: showDuckDuckGo(); break;
  case MENU_ACTION_WIFI_SURVEY: showWifiSurvey(); break;
  case MENU_ACTION_BINARY_CLOCK:
  case MENU_ACTION_UNIX_TIME:
  case MENU_ACTION_UTC_CLOCK:
  case MENU_ACTION_WEEK_NUMBER:
  case MENU_ACTION_DAY_OF_YEAR:
  case MENU_ACTION_MONTH_CALENDAR:
  case MENU_ACTION_WORLD_CLOCKS:
  case MENU_ACTION_DUAL_TIME:
  case MENU_ACTION_INTERNET_BEATS:
  case MENU_ACTION_DECIMAL_TIME:
  case MENU_ACTION_JULIAN_DAY:
  case MENU_ACTION_TIME_PROGRESS:
    showClockTool(static_cast<uint8_t>(action) -
                  static_cast<uint8_t>(MENU_ACTION_BINARY_CLOCK));
    break;
  case MENU_ACTION_STOPWATCH:
  case MENU_ACTION_COUNTDOWN:
  case MENU_ACTION_DAILY_ALARM:
  case MENU_ACTION_POMODORO:
  case MENU_ACTION_INTERVAL_TIMER:
  case MENU_ACTION_METRONOME:
    showTimerTool(static_cast<uint8_t>(action) -
                  static_cast<uint8_t>(MENU_ACTION_STOPWATCH));
    break;
  case MENU_ACTION_BATTERY_GAUGE:
  case MENU_ACTION_POWER_BUDGET:
  case MENU_ACTION_CHARGE_STATUS:
  case MENU_ACTION_BMA_TEMPERATURE:
  case MENU_ACTION_RAW_ACCEL:
  case MENU_ACTION_G_FORCE:
  case MENU_ACTION_SPIRIT_LEVEL:
  case MENU_ACTION_ORIENTATION:
  case MENU_ACTION_MOTION_SCORE:
  case MENU_ACTION_STEP_COUNTER:
  case MENU_ACTION_STEP_GOAL:
  case MENU_ACTION_WALK_DISTANCE:
  case MENU_ACTION_STEP_CALORIES:
  case MENU_ACTION_ACTIVITY_STATE:
  case MENU_ACTION_SENSOR_STATUS:
  case MENU_ACTION_UPTIME:
  case MENU_ACTION_SHAKE_COUNTER:
    showSensorTool(static_cast<uint8_t>(action) -
                   static_cast<uint8_t>(MENU_ACTION_BATTERY_GAUGE));
    break;
  case MENU_ACTION_BLE_SCANNER:
  case MENU_ACTION_BLE_DEVICE_COUNT:
  case MENU_ACTION_BLE_STRONGEST:
  case MENU_ACTION_BLE_NAMED:
  case MENU_ACTION_BLE_SERVICES:
  case MENU_ACTION_BLE_MANUFACTURERS:
  case MENU_ACTION_BLE_RSSI_BANDS:
  case MENU_ACTION_BLE_ADDRESSES:
  case MENU_ACTION_BLE_RADAR:
  case MENU_ACTION_BLE_TX_POWER:
  case MENU_ACTION_BLE_IBEACONS:
  case MENU_ACTION_BLE_BEACON:
  case MENU_ACTION_BLE_BATTERY_BEACON:
  case MENU_ACTION_BLE_TIME_BEACON:
  case MENU_ACTION_BLE_STEP_BEACON:
  case MENU_ACTION_BLE_NAME_BADGE:
    showBluetoothTool(static_cast<uint8_t>(action) -
                      static_cast<uint8_t>(MENU_ACTION_BLE_SCANNER));
    break;
  case MENU_ACTION_EMERGENCY_PLATE:
  case MENU_ACTION_MEDICAL_ID:
  case MENU_ACTION_ICE_CONTACT:
  case MENU_ACTION_BLOOD_TYPE:
  case MENU_ACTION_ALLERGIES:
  case MENU_ACTION_MEDICATIONS:
  case MENU_ACTION_CONDITIONS:
  case MENU_ACTION_EDIT_MEDICAL_ID:
    showHealthcareTool(static_cast<uint8_t>(action) -
                       static_cast<uint8_t>(MENU_ACTION_EMERGENCY_PLATE));
    break;
  case MENU_ACTION_FALL_DETECTOR:
  case MENU_ACTION_BODY_POSITION:
  case MENU_ACTION_CONFIGURED_LOCATION:
  case MENU_ACTION_SOS_SCREEN:
  case MENU_ACTION_SOS_BLE:
    showSafetyTool(static_cast<uint8_t>(action) -
                   static_cast<uint8_t>(MENU_ACTION_FALL_DETECTOR));
    break;
  case MENU_ACTION_CHECK_IN_TIMER:
  case MENU_ACTION_MEDICATION_REMINDER:
  case MENU_ACTION_HYDRATION_REMINDER:
    showHealthReminder(static_cast<uint8_t>(action) -
                       static_cast<uint8_t>(MENU_ACTION_CHECK_IN_TIMER));
    break;
  case MENU_ACTION_BREATHING_COACH:
  case MENU_ACTION_CPR_METRONOME:
  case MENU_ACTION_RECOVERY_POSITION:
  case MENU_ACTION_STROKE_FAST:
  case MENU_ACTION_CHOKING_RESPONSE:
  case MENU_ACTION_SEIZURE_AID:
  case MENU_ACTION_SEVERE_BLEEDING:
  case MENU_ACTION_BURN_FIRST_AID:
  case MENU_ACTION_HEAT_EMERGENCY:
  case MENU_ACTION_HYPOTHERMIA:
  case MENU_ACTION_POISONING:
  case MENU_ACTION_ANAPHYLAXIS:
  case MENU_ACTION_OPIOID_OVERDOSE:
  case MENU_ACTION_ASTHMA_ATTACK:
  case MENU_ACTION_EMERGENCY_NUMBERS:
  case MENU_ACTION_PAIN_LOG:
  case MENU_ACTION_SYMPTOM_NOTE:
  case MENU_ACTION_GROUNDING:
    showHealthSupportTool(static_cast<uint8_t>(action) -
                          static_cast<uint8_t>(MENU_ACTION_BREATHING_COACH));
    break;
  case MENU_ACTION_COIN_FLIP:
  case MENU_ACTION_D6_DICE:
  case MENU_ACTION_D20_DICE:
  case MENU_ACTION_RANDOM_NUMBER:
  case MENU_ACTION_DECISION_MAKER:
  case MENU_ACTION_PASSWORD_GENERATOR:
  case MENU_ACTION_UUID_GENERATOR:
  case MENU_ACTION_I2C_SCANNER:
  case MENU_ACTION_CHIP_INFO:
  case MENU_ACTION_HEAP_MONITOR:
  case MENU_ACTION_WAKE_REASON:
  case MENU_ACTION_RESET_REASON:
  case MENU_ACTION_BUTTON_TESTER:
  case MENU_ACTION_VIBRATION_LAB:
  case MENU_ACTION_SCREEN_RULER:
  case MENU_ACTION_TEMPERATURE_CONVERTER:
  case MENU_ACTION_LENGTH_CONVERTER:
  case MENU_ACTION_WEIGHT_CONVERTER:
  case MENU_ACTION_BASE_CONVERTER:
  case MENU_ACTION_PACE_CONVERTER:
    showUtilityTool(static_cast<uint8_t>(action) -
                    static_cast<uint8_t>(MENU_ACTION_COIN_FLIP));
    break;
  case MENU_ACTION_ROCK_PAPER_SCISSORS:
  case MENU_ACTION_REACTION_TEST:
  case MENU_ACTION_HIGHER_LOWER:
  case MENU_ACTION_NUMBER_GUESS:
  case MENU_ACTION_NIM:
  case MENU_ACTION_TIC_TAC_TOE:
  case MENU_ACTION_LIGHTS_OUT:
  case MENU_ACTION_BLACKJACK:
  case MENU_ACTION_QUICK_MATH:
  case MENU_ACTION_BALANCE_CHALLENGE:
    showMiniGame(static_cast<uint8_t>(action) -
                 static_cast<uint8_t>(MENU_ACTION_ROCK_PAPER_SCISSORS));
    break;
  }
}

void Watchy::leaveMenuLevel() {
  if (menuLevel == MENU_LEVEL_APPLICATIONS) {
    submenuMenuIndices[menuCategory] = menuIndex;
    menuLevel = MENU_LEVEL_CATEGORIES;
    menuIndex = categoryMenuIndex;
    showMenu(menuIndex, false);
    return;
  }

  RTC.read(currentTime);
  showWatchFace();
}

void Watchy::moveMenuSelection(int direction, bool fastRefresh) {
  int itemCount = menuLevel == MENU_LEVEL_CATEGORIES
                      ? getMenuCategoryCount()
                      : getMenuItemCount(menuCategory);
  if (itemCount <= 0) {
    return;
  }
  int previousIndex = menuIndex;
  menuIndex = (menuIndex + direction + itemCount) % itemCount;
  if (menuLevel == MENU_LEVEL_CATEGORIES) {
    categoryMenuIndex = menuIndex;
  } else {
    submenuMenuIndices[menuCategory] = menuIndex;
  }
  if (fastRefresh) {
    showFastMenu(menuIndex, previousIndex);
  } else {
    showMenu(menuIndex, true);
  }
}

void Watchy::vibMotor(uint16_t intervalMs, uint8_t length) {
  pinMode(VIB_MOTOR_PIN, OUTPUT);
  digitalWrite(VIB_MOTOR_PIN, LOW);
  bool motorOn = false;
  for (int i = 0; i < length; i++) {
    motorOn = !motorOn;
    digitalWrite(VIB_MOTOR_PIN, motorOn);
    WatchyUi::deepSleepDelay(intervalMs);
  }
  digitalWrite(VIB_MOTOR_PIN, LOW);
}

weatherData Watchy::getWeatherData() {
  return _getWeatherData(settings.cityID, settings.lat, settings.lon,
    settings.weatherUnit, settings.weatherLang, settings.weatherURL,
    settings.weatherAPIKey, settings.weatherUpdateInterval);
}

weatherData Watchy::getCachedWeatherData() const {
  return currentWeather;
}

weatherData Watchy::_getWeatherData(String cityID, String lat, String lon, String units, String lang,
                                   String url, String apiKey,
                                   uint8_t updateInterval) {
  currentWeather.isMetric = units == String("metric");
  if (weatherIntervalCounter < 0) { //-1 on first run, set to updateInterval
    weatherIntervalCounter = updateInterval;
  }
  if (weatherIntervalCounter >=
      updateInterval) { // only update if WEATHER_UPDATE_INTERVAL has elapsed
                        // i.e. 30 minutes
    weatherIntervalCounter = 0;
    if (connectWiFi()) {
      HTTPClient http; // Use Weather API for live data if WiFi is connected
      http.setConnectTimeout(3000); // 3 second max timeout
      String weatherQueryURL = url;
      if(cityID != ""){
        weatherQueryURL.replace("{cityID}", cityID);
      }else{
        weatherQueryURL.replace("{lat}", lat);
        weatherQueryURL.replace("{lon}", lon);
      }
      weatherQueryURL.replace("{units}", units);
      weatherQueryURL.replace("{lang}", lang);
      weatherQueryURL.replace("{apiKey}", apiKey);
      http.begin(weatherQueryURL.c_str());
      int httpResponseCode = http.GET();
      if (httpResponseCode == 200) {
        String payload             = http.getString();
        JSONVar responseObject     = JSON.parse(payload);
        currentWeather.temperature = int(responseObject["main"]["temp"]);
        currentWeather.weatherConditionCode =
            int(responseObject["weather"][0]["id"]);
	      currentWeather.external = true;
		        breakTime((time_t)(int)responseObject["sys"]["sunrise"], currentWeather.sunrise);
		        breakTime((time_t)(int)responseObject["sys"]["sunset"], currentWeather.sunset);
        // sync NTP during weather API call and use timezone of lat & lon
        gmtOffset = int(responseObject["timezone"]);
        syncNTP(gmtOffset);
      } else {
        // http error
      }
      http.end();
      // turn off radios
      WiFi.mode(WIFI_OFF);
      btStop();
      WatchyDiagnostics::endWifiSession();
      setLowPowerCpuFrequency();
    } else { // No WiFi, use internal temperature sensor
      uint8_t temperature = sensor.readTemperature(); // celsius
      if (!currentWeather.isMetric) {
        temperature = temperature * 9. / 5. + 32.; // fahrenheit
      }
      currentWeather.temperature          = temperature;
      currentWeather.weatherConditionCode = 800;
      currentWeather.external             = false;
    }
  } else {
    weatherIntervalCounter++;
  }
  return currentWeather;
}

float Watchy::getBatteryVoltage() {
  #ifdef ARDUINO_ESP32S3_DEV
    return analogReadMilliVolts(BATT_ADC_PIN) / 1000.0f * ADC_VOLTAGE_DIVIDER;
  #else
  if (RTC.rtcType == DS3231) {
    return analogReadMilliVolts(BATT_ADC_PIN) / 1000.0f *
           2.0f; // Battery voltage goes through a 1/2 divider.
  } else {
    return analogReadMilliVolts(BATT_ADC_PIN) / 1000.0f * 2.0f;
  }
  #endif
}

uint8_t Watchy::getBoardRevision() {
  esp_chip_info_t chip_info;
  esp_chip_info(&chip_info);
  if(chip_info.model == CHIP_ESP32){ //Revision 1.0 - 2.0
    Wire.beginTransmission(0x68); //v1.0 has DS3231
    if (Wire.endTransmission() == 0){
      return 10;
    }
    WatchyUi::deepSleepDelay(1);
    Wire.beginTransmission(0x51); //v1.5 and v2.0 have PCF8563
    if (Wire.endTransmission() == 0){
        pinMode(35, INPUT);
        if(digitalRead(35) == 0){
          return 20; //in rev 2.0, pin 35 is BTN 3 and has a pulldown
        }else{
          return 15; //in rev 1.5, pin 35 is the battery ADC
        }
    }
  }
  if(chip_info.model == CHIP_ESP32S3){ //Revision 3.0
    return 30;
  }
  return -1;
}

uint16_t Watchy::_readRegister(uint8_t address, uint8_t reg, uint8_t *data,
                               uint16_t len) {
  Wire.beginTransmission(address);
  Wire.write(reg);
  if (Wire.endTransmission() != 0) {
    return 1;
  }
  uint16_t requested = min<uint16_t>(len, UINT8_MAX);
  Wire.requestFrom(address, static_cast<uint8_t>(requested));
  uint16_t index = 0;
  while (Wire.available() && index < requested) {
    data[index++] = Wire.read();
  }
  while (Wire.available()) {
    Wire.read();
  }
  return index == len ? 0 : 1;
}

uint16_t Watchy::_writeRegister(uint8_t address, uint8_t reg, uint8_t *data,
                                uint16_t len) {
  Wire.beginTransmission(address);
  Wire.write(reg);
  Wire.write(data, len);
  return (0 != Wire.endTransmission());
}

void Watchy::_bmaConfig() {

  if (sensor.begin(_readRegister, _writeRegister, sensorDelay) == false) {
    // fail to init BMA
    return;
  }

    WatchySensor::initializeBaseline();

  struct bma4_int_pin_config config;
  config.edge_ctrl = BMA4_LEVEL_TRIGGER;
  config.lvl       = BMA4_ACTIVE_HIGH;
  config.od        = BMA4_PUSH_PULL;
  config.output_en = BMA4_OUTPUT_ENABLE;
  config.input_en  = BMA4_INPUT_DISABLE;
  // The correct trigger interrupt needs to be configured as needed
  sensor.setINTPinConfig(config, BMA4_INTR1_MAP);

  struct bma423_axes_remap remap_data;
  remap_data.x_axis      = 1;
  remap_data.x_axis_sign = 0xFF;
  remap_data.y_axis      = 0;
  remap_data.y_axis_sign = 0xFF;
  remap_data.z_axis      = 2;
  remap_data.z_axis_sign = 0xFF;
  // Need to raise the wrist function, need to set the correct axis
  sensor.setRemapAxes(&remap_data);

  // Enable BMA423 isStepCounter feature
  sensor.enableFeature(BMA423_STEP_CNTR, true);
  // Reset steps
  sensor.resetStepCounter();
}
