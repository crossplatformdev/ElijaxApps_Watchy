#include "Watchy.h"
#include "app/HeartRate.h"
#include "app/HealthcareAlerts.h"
#include "app/TimeTools.h"
#include "os/MenuModel.h"
#include "sdk/WatchyUi.h"
#include "esp32-hal-cpu.h"
#include <type_traits>

namespace {

constexpr uint32_t LOW_POWER_CPU_FREQUENCY_MHZ = 40;
constexpr uint32_t RADIO_CPU_FREQUENCY_MHZ = 80;

void lowPowerDelay(uint32_t durationMs) {
#ifdef ARDUINO_ESP32S3_DEV
  if (esp_sleep_enable_timer_wakeup(
          static_cast<uint64_t>(durationMs) * 1000ULL) != ESP_OK) {
    delay(durationMs);
    return;
  }
  esp_err_t sleepResult = esp_light_sleep_start();
  esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_TIMER);
  if (sleepResult != ESP_OK) {
    delay(durationMs);
  }
#else
  delay(durationMs);
#endif
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

void Watchy::init(String datetime) {
  setLowPowerCpuFrequency();
  WatchyUi::Theme::load();
  if (guiState < WATCHFACE_STATE || guiState > FW_UPDATE_STATE) {
    guiState = WATCHFACE_STATE;
  }
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause(); // get wake up reason
  #ifdef ARDUINO_ESP32S3_DEV
    Wire.begin(WATCHY_V3_SDA, WATCHY_V3_SCL);     // init i2c
  #else
    Wire.begin(SDA, SCL);                         // init i2c
  #endif
  RTC.init();
  // Init the display since is almost sure we will use it
  display.epd2.initWatchy(wakeup_reason == ESP_SLEEP_WAKEUP_UNDEFINED);

  switch (wakeup_reason) {
  #ifdef ARDUINO_ESP32S3_DEV
  case ESP_SLEEP_WAKEUP_TIMER: // RTC Alarm
  #else
  case ESP_SLEEP_WAKEUP_EXT0: // RTC Alarm
  #endif
    RTC.read(currentTime);
    checkDailyAlarm(*this, currentTime);
    HealthcareAlerts::check(*this, currentTime);
    {
    bool heartRateRefresh = serviceWatchfaceHeartRateMonitoring();
    switch (guiState) {
    case WATCHFACE_STATE:
      {
      bool minuteRefresh =
          currentTime.Second < HEART_RATE_BACKGROUND_WAKE_SECONDS;
      if (!isWatchfaceHeartRateMonitoringActive() || heartRateRefresh ||
          minuteRefresh) {
        showWatchFace(!isWatchfaceHeartRateMonitoringActive() ||
                      minuteRefresh);
      }
      if (settings.vibrateOClock) {
        if (currentTime.Minute == 0 && minuteRefresh) {
          vibMotor(75, 4);
        }
      }
      }
      break;
    case MAIN_MENU_STATE:
      // Return to watchface if in menu for more than one tick
      if (alreadyInMenu) {
        guiState = WATCHFACE_STATE;
        showWatchFace();
      } else {
        alreadyInMenu = true;
      }
      break;
    }
    }
    break;
  case ESP_SLEEP_WAKEUP_EXT1: // button Press
    handleButtonPress();
    break;
  #ifdef ARDUINO_ESP32S3_DEV
  case ESP_SLEEP_WAKEUP_EXT0: // USB plug in
    pinMode(USB_DET_PIN, INPUT);
    USB_PLUGGED_IN = (digitalRead(USB_DET_PIN) == 1);
    if(guiState == WATCHFACE_STATE){
      RTC.read(currentTime);
      showWatchFace();
    }
    break;
  #endif
  default: // reset
    RTC.config(datetime);
    _bmaConfig();
    setWatchfaceHeartRateMonitoring(false);
    #ifdef ARDUINO_ESP32S3_DEV
    pinMode(USB_DET_PIN, INPUT);
    USB_PLUGGED_IN = (digitalRead(USB_DET_PIN) == 1);
    #endif    
    gmtOffset = settings.gmtOffset;
    RTC.read(currentTime);
    RTC.read(bootTime);
    connectWiFi();
    WiFi.disconnect(true, false);
    btStop();
    setLowPowerCpuFrequency();
    showWatchFace();
    vibMotor(75, 4);
    // For some reason, seems to be enabled on first boot
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
    break;
  }
  deepSleep();
}
void Watchy::deepSleep() {
  stopHeartRateMeasurement();
  waitForHeartRateMeasurement();
  if (guiState != WATCHFACE_STATE) {
    setWatchfaceHeartRateMonitoring(false);
  }
  WiFi.disconnect(true, false);
  btStop();
  BLE_CONFIGURED = false;
  setLowPowerCpuFrequency();
  display.hibernate();
  RTC.clearAlarm();        // resets the alarm flag in the RTC
  #ifdef ARDUINO_ESP32S3_DEV
  esp_sleep_enable_ext0_wakeup((gpio_num_t)USB_DET_PIN, USB_PLUGGED_IN ? LOW : HIGH); //// enable deep sleep wake on USB plug in/out
  rtc_gpio_set_direction((gpio_num_t)USB_DET_PIN, RTC_GPIO_MODE_INPUT_ONLY);
  rtc_gpio_pullup_en((gpio_num_t)USB_DET_PIN);

  esp_sleep_enable_ext1_wakeup(
      BTN_PIN_MASK,
      ESP_EXT1_WAKEUP_ANY_LOW); // enable deep sleep wake on button press
  rtc_gpio_set_direction((gpio_num_t)UP_BTN_PIN, RTC_GPIO_MODE_INPUT_ONLY);
  rtc_gpio_pullup_en((gpio_num_t)UP_BTN_PIN);

  rtc_clk_32k_enable(false);
  RTC.read(currentTime);
  uint8_t second = currentTime.Second < 60 ? currentTime.Second : 0;
    uint8_t wakeInterval = isWatchfaceHeartRateMonitoringActive()
                 ? HEART_RATE_BACKGROUND_WAKE_SECONDS
                 : 60;
    uint8_t secondsToNextWake = wakeInterval - second % wakeInterval;
    esp_sleep_enable_timer_wakeup(
      static_cast<uint64_t>(secondsToNextWake) * uS_TO_S_FACTOR);
  #else
  // Set GPIOs 0-39 to input to avoid power leaking out
  const uint64_t ignore = 0b11110001000000110000100111000010; // Ignore some GPIOs due to resets
  for (int i = 0; i < GPIO_NUM_MAX; i++) {
    if ((ignore >> i) & 0b1)
      continue;
    pinMode(i, INPUT);
  }
  esp_sleep_enable_ext0_wakeup((gpio_num_t)RTC_INT_PIN,
                               0); // enable deep sleep wake on RTC interrupt
  esp_sleep_enable_ext1_wakeup(
      BTN_PIN_MASK,
      ESP_EXT1_WAKEUP_ANY_HIGH); // enable deep sleep wake on button press
  #endif
  esp_deep_sleep_start();
}

void Watchy::handleButtonPress() {
  uint64_t wakeupBit = esp_sleep_get_ext1_wakeup_status();
  if (wakeupBit & MENU_BTN_MASK) {
    if (guiState == WATCHFACE_STATE) {
      setWatchfaceHeartRateMonitoring(false);
      menuLevel = MENU_LEVEL_CATEGORIES;
      menuIndex = categoryMenuIndex;
      showMenu(menuIndex, false);
    } else if (guiState == MAIN_MENU_STATE) {
      selectMenuEntry();
    }
  } else if (wakeupBit & BACK_BTN_MASK) {
    if (guiState == MAIN_MENU_STATE) {
      leaveMenuLevel();
    } else if (guiState == APP_STATE || guiState == FW_UPDATE_STATE) {
      menuLevel = MENU_LEVEL_APPLICATIONS;
      menuIndex = submenuMenuIndices[menuCategory];
      showMenu(menuIndex, false);
    }
  } else if (wakeupBit & UP_BTN_MASK) {
    if (guiState == MAIN_MENU_STATE) {
      moveMenuSelection(-1, false);
    }
  } else if (wakeupBit & DOWN_BTN_MASK) {
    if (guiState == MAIN_MENU_STATE) {
      moveMenuSelection(1, false);
    }
  }

  /***************** fast menu *****************/
  bool timeout     = false;
  long lastTimeout = millis();
  pinMode(MENU_BTN_PIN, INPUT);
  pinMode(BACK_BTN_PIN, INPUT);
  pinMode(UP_BTN_PIN, INPUT);
  pinMode(DOWN_BTN_PIN, INPUT);
  while (!timeout) {
    if (millis() - lastTimeout > 5000) {
      timeout = true;
    } else {
      if (digitalRead(MENU_BTN_PIN) == ACTIVE_LOW) {
        lastTimeout = millis();
        if (guiState == MAIN_MENU_STATE) {
          selectMenuEntry();
        }
      } else if (digitalRead(BACK_BTN_PIN) == ACTIVE_LOW) {
        lastTimeout = millis();
        if (guiState == MAIN_MENU_STATE) {
          leaveMenuLevel();
          if (guiState == WATCHFACE_STATE) {
            break;
          }
        } else if (guiState == APP_STATE || guiState == FW_UPDATE_STATE) {
          menuLevel = MENU_LEVEL_APPLICATIONS;
          menuIndex = submenuMenuIndices[menuCategory];
          showMenu(menuIndex, false);
        }
      } else if (digitalRead(UP_BTN_PIN) == ACTIVE_LOW) {
        lastTimeout = millis();
        if (guiState == MAIN_MENU_STATE) {
          moveMenuSelection(-1, true);
        }
      } else if (digitalRead(DOWN_BTN_PIN) == ACTIVE_LOW) {
        lastTimeout = millis();
        if (guiState == MAIN_MENU_STATE) {
          moveMenuSelection(1, true);
        }
      }
      lowPowerDelay(20);
    }
  }
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
  menuIndex = (menuIndex + direction + itemCount) % itemCount;
  if (menuLevel == MENU_LEVEL_CATEGORIES) {
    categoryMenuIndex = menuIndex;
  } else {
    submenuMenuIndices[menuCategory] = menuIndex;
  }
  if (fastRefresh) {
    showFastMenu(menuIndex);
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
    delay(intervalMs);
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
    delay(1);
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

  if (sensor.begin(_readRegister, _writeRegister, delay) == false) {
    // fail to init BMA
    return;
  }

  // Accel parameter structure
  Acfg cfg;
  /*!
      Output data rate in Hz, Optional parameters:
          - BMA4_OUTPUT_DATA_RATE_0_78HZ
          - BMA4_OUTPUT_DATA_RATE_1_56HZ
          - BMA4_OUTPUT_DATA_RATE_3_12HZ
          - BMA4_OUTPUT_DATA_RATE_6_25HZ
          - BMA4_OUTPUT_DATA_RATE_12_5HZ
          - BMA4_OUTPUT_DATA_RATE_25HZ
          - BMA4_OUTPUT_DATA_RATE_50HZ
          - BMA4_OUTPUT_DATA_RATE_100HZ
          - BMA4_OUTPUT_DATA_RATE_200HZ
          - BMA4_OUTPUT_DATA_RATE_400HZ
          - BMA4_OUTPUT_DATA_RATE_800HZ
          - BMA4_OUTPUT_DATA_RATE_1600HZ
  */
  cfg.odr = BMA4_OUTPUT_DATA_RATE_100HZ;
  /*!
      G-range, Optional parameters:
          - BMA4_ACCEL_RANGE_2G
          - BMA4_ACCEL_RANGE_4G
          - BMA4_ACCEL_RANGE_8G
          - BMA4_ACCEL_RANGE_16G
  */
  cfg.range = BMA4_ACCEL_RANGE_2G;
  /*!
      Bandwidth parameter, determines filter configuration, Optional parameters:
          - BMA4_ACCEL_OSR4_AVG1
          - BMA4_ACCEL_OSR2_AVG2
          - BMA4_ACCEL_NORMAL_AVG4
          - BMA4_ACCEL_CIC_AVG8
          - BMA4_ACCEL_RES_AVG16
          - BMA4_ACCEL_RES_AVG32
          - BMA4_ACCEL_RES_AVG64
          - BMA4_ACCEL_RES_AVG128
  */
  cfg.bandwidth = BMA4_ACCEL_NORMAL_AVG4;

  /*! Filter performance mode , Optional parameters:
      - BMA4_CIC_AVG_MODE
      - BMA4_CONTINUOUS_MODE
  */
  cfg.perf_mode = BMA4_CONTINUOUS_MODE;

  // Configure the BMA423 accelerometer
  sensor.setAccelConfig(cfg);

  // Enable BMA423 accelerometer
  // Warning : Need to use feature, you must first enable the accelerometer
  // Warning : Need to use feature, you must first enable the accelerometer
  sensor.enableAccel();

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
  // Enable BMA423 isTilt feature
  sensor.enableFeature(BMA423_TILT, true);
  // Enable BMA423 isDoubleClick feature
  sensor.enableFeature(BMA423_WAKEUP, true);

  // Reset steps
  sensor.resetStepCounter();

  // Turn on feature interrupt
  sensor.enableStepCountInterrupt();
  sensor.enableTiltInterrupt();
  // It corresponds to isDoubleClick interrupt
  sensor.enableWakeupInterrupt();
}

void Watchy::_configModeCallback(WiFiManager *myWiFiManager) {
  (void)myWiFiManager;
  display.setFullWindow();
  display.fillScreen(DARKMODE ? GxEPD_BLACK : GxEPD_WHITE);
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
  display.setCursor(0, 22);
  display.println("WIFI SETUP");
  display.println();
  display.println("Join: Watchy AP");
  display.println("Open browser:");
  display.println("192.168.4.1");
  display.println();
  display.println("BACK to exit");
  display.display(true); // partial refresh
}

/*
void Watchy::showUpdateFW() {
  display.setFullWindow();
  display.fillScreen(GxEPD_BLACK);
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);;
  display.setCursor(0, 30);
  display.println("Please visit");
  display.println("watchy.sqfmi.com");
  display.println("with a Bluetooth");
  display.println("enabled device");
  display.println(" ");
  display.println("Press menu button");
  display.println("again when ready");
  display.println(" ");
  display.println("Keep USB powered");
  display.display(true); // partial refresh

  guiState = FW_UPDATE_STATE;
}

void Watchy::updateFWBegin() {
  display.setFullWindow();
  display.fillScreen(GxEPD_BLACK);
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);;
  display.setCursor(0, 30);
  display.println("Bluetooth Started");
  display.println(" ");
  display.println("Watchy BLE OTA");
  display.println(" ");
  display.println("Waiting for");
  display.println("connection...");
  display.display(true); // partial refresh

  BLE BT;
  BT.begin("Watchy BLE OTA");
  int prevStatus = -1;
  int currentStatus;

  while (1) {
    currentStatus = BT.updateStatus();
    if (prevStatus != currentStatus || prevStatus == 1) {
      if (currentStatus == 0) {
        display.setFullWindow();
        display.fillScreen(DARKMODE ? GxEPD_BLACK : GxEPD_WHITE);

        display.setFont(&FreeMonoBold9pt7b);
        display.setTextColor(DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);;
        display.setCursor(0, 30);
        display.println("BLE Connected!");
        display.println(" ");
        display.println("Waiting for");
        display.println("upload...");
        display.display(true); // partial refresh
      }
      if (currentStatus == 1) {
        display.setFullWindow();
        display.fillScreen(DARKMODE ? GxEPD_BLACK : GxEPD_WHITE);

        display.setFont(&FreeMonoBold9pt7b);
        display.setTextColor(DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);;
        display.setCursor(0, 30);
        display.println("Downloading");
        display.println("firmware:");
        display.println(" ");
        display.print(BT.howManyBytes());
        display.println(" bytes");
        display.display(true); // partial refresh
      }
      if (currentStatus == 2) {
        display.setFullWindow();
        display.fillScreen(DARKMODE ? GxEPD_BLACK : GxEPD_WHITE);

        display.setFont(&FreeMonoBold9pt7b);
        display.setTextColor(DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);;
        display.setCursor(0, 30);
        display.println("Download");
        display.println("completed!");
        display.println(" ");
        display.println("Rebooting...");
        display.display(true); // partial refresh

        delay(2000);
        esp_restart();
      }
      if (currentStatus == 4) {
        display.setFullWindow();
        display.fillScreen(DARKMODE ? GxEPD_BLACK : GxEPD_WHITE);

        display.setFont(&FreeMonoBold9pt7b);
        display.setTextColor(DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);;
        display.setCursor(0, 30);
        display.println("BLE Disconnected!");
        display.println(" ");
        display.println("exiting...");
        display.display(true); // partial refresh
        delay(1000);
        break;
      }
      prevStatus = currentStatus;
    }
    delay(100);
  }

  // turn off radios
  WiFi.mode(WIFI_OFF);
  btStop();
  showMenu(menuIndex, false);
}
*/

void Watchy::drawWatchFace() {
  display.setFont(&DSEG7_Classic_Bold_53);
  display.setCursor(5, 53 + 60);
  if (currentTime.Hour < 10) {
    display.print("0");
  }
  display.print(currentTime.Hour);
  display.print(":");
  if (currentTime.Minute < 10) {
    display.print("0");
  }
  display.println(currentTime.Minute);
}

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
