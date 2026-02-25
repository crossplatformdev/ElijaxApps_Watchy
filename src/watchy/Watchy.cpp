#include "Watchy.h"
#include "../settings/settings.h"
#include "../sdk/UiSDK.h"
#include "../sdk/UiTemplates.h"

#ifdef ARDUINO_ESP32S3_DEV
Watchy32KRTC Watchy::RTC;
#else
WatchyRTC Watchy::RTC;
#endif

ThemeableGxEPD2_BW<WatchyDisplay, WatchyDisplay::HEIGHT> Watchy::display(WatchyDisplay{});

RTC_DATA_ATTR int guiState;
RTC_DATA_ATTR int menuIndex;
RTC_DATA_ATTR BMA423 sensor;
RTC_DATA_ATTR bool WIFI_CONFIGURED;
RTC_DATA_ATTR bool BLE_CONFIGURED;
RTC_DATA_ATTR weatherData currentWeather;
RTC_DATA_ATTR int weatherIntervalCounter = -1;
RTC_DATA_ATTR long gmtOffset = GMT_OFFSET_SEC;
RTC_DATA_ATTR bool alreadyInMenu = true;
RTC_DATA_ATTR bool USB_PLUGGED_IN = false;
RTC_DATA_ATTR tmElements_t bootTime;
RTC_DATA_ATTR uint32_t lastIPAddress;
RTC_DATA_ATTR char lastSSID[30];
RTC_DATA_ATTR char lastPassword[64];
RTC_DATA_ATTR uint8_t currentWatchfaceId = 0;
RTC_DATA_ATTR bool gDarkMode = 0;      // runtime theme (true=dark background)
RTC_DATA_ATTR uint8_t menuLevel = 0;     // 0 = top-level categories, 1 = submenu
RTC_DATA_ATTR uint8_t menuCategory = 0;  // active top-level category when in submenu
RTC_DATA_ATTR uint32_t lastDeepSleepTime = millis();

namespace {
Watchy::ButtonHandler sBackHandler = nullptr;
Watchy::ButtonHandler sUpHandler = nullptr;
Watchy::ButtonHandler sMenuHandler = nullptr;
Watchy::ButtonHandler sDownHandler = nullptr;
} // namespace

void Watchy::setButtonHandlers(ButtonHandler back,
                               ButtonHandler up,
                               ButtonHandler menu,
                               ButtonHandler down) {
  sBackHandler = back;
  sUpHandler = up;
  sMenuHandler = menu;
  sDownHandler = down;
}

void Watchy::clearButtonHandlers() {
  setButtonHandlers(nullptr, nullptr, nullptr, nullptr);
}

void Watchy::backPressed() {
  if (UiTemplates::isCapturingControls()) {
    UiTemplates::postControlEvent(UiTemplates::ControlEvent::Back);
    return;
  }
  if (sBackHandler) {
    sBackHandler(this);
  }
}

void Watchy::menuPressed() {
  if (UiTemplates::isCapturingControls()) {
    UiTemplates::postControlEvent(UiTemplates::ControlEvent::Accept);
    return;
  }
  if (sMenuHandler) {
    sMenuHandler(this);
  }
}

void Watchy::upPressed() {
  if (UiTemplates::isCapturingControls()) {
    UiTemplates::postControlEvent(UiTemplates::ControlEvent::Up);
    return;
  }
  if (sUpHandler) {
    sUpHandler(this);
  }
}

void Watchy::downPressed() {
  if (UiTemplates::isCapturingControls()) {
    UiTemplates::postControlEvent(UiTemplates::ControlEvent::Down);
    return;
  }
  if (sDownHandler) {
    sDownHandler(this);
  }
}

void Watchy::init(String datetime)
{
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause(); // get wake up reason
#ifdef ARDUINO_ESP32S3_DEV
  Wire.begin(WATCHY_V3_SDA, WATCHY_V3_SCL); // init i2c
#else
  Wire.begin(SDA, SCL); // init i2c
#endif
  RTC.init();
  // Init the display since is almost sure we will use it
  display.epd2.initWatchy();
  switch (wakeup_reason)
  {
#ifdef ARDUINO_ESP32S3_DEV
  case ESP_SLEEP_WAKEUP_TIMER: // RTC Alarm
#else
  case ESP_SLEEP_WAKEUP_EXT0: // RTC Alarm
#endif
    RTC.read(currentTime);
    switch (guiState)
    {
    default:
    case WATCHFACE_STATE:
      showWatchFace(true); // partial updates on tick
      checkAlarmTrigger();
      if (settings.vibrateOClock)
      {
        if (currentTime.Minute == 0)
        {
          // The RTC wakes us up once per minute
          vibMotor(75, 4);
        }
      }
      break;
    case MAIN_MENU_STATE:
      // Return to watchface if in menu for more than one tick
      if (alreadyInMenu)
      {
        guiState = WATCHFACE_STATE;
        showWatchFace(true);
      }
      else
      {
        alreadyInMenu = true;
      }
      break;
    }
    break;
  case ESP_SLEEP_WAKEUP_EXT1: // button Press
    handleButtonPress();
    break;
#ifdef ARDUINO_ESP32S3_DEV
  case ESP_SLEEP_WAKEUP_EXT0: // USB plug in
    pinMode(USB_DET_PIN, INPUT);
    USB_PLUGGED_IN = (digitalRead(USB_DET_PIN) == 1);
    // Always show a stable screen when USB toggles, regardless of prior state
    RTC.read(currentTime);
    guiState = WATCHFACE_STATE;
    alreadyInMenu = false;
    showWatchFace(false); // full refresh to avoid partial artifacts on plug/unplug
    break;
#endif
  default: // reset
    RTC.config(datetime); // set RTC time if reset
    _bmaConfig();
    sensor.enableAccel(); // enable step counter on reset
#ifdef ARDUINO_ESP32S3_DEV
    pinMode(USB_DET_PIN, INPUT);
    USB_PLUGGED_IN = (digitalRead(USB_DET_PIN) == 1);
#endif
    RTC.read(currentTime);
    RTC.read(bootTime);
    guiState      = WATCHFACE_STATE;
    alreadyInMenu = false;
    //menuLevel     = 0;
    //menuCategory  = 0;
    //menuIndex     = 0;
    // Full refresh on cold boot to ensure foreground is visible and avoid
    // partial-refresh artifacts that can leave the screen blank.
    showWatchFace(false);
    vibMotor(75, 4);
    // For some reason, seems to be enabled on first boot
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
    break;
  }
  deepSleep();
}
void Watchy::deepSleep()
{
  setCpuFrequencyMhz(80); // run at 80MHz on boot to save power until we need WiFi
  lastDeepSleepTime = millis();
  // Refresh cached USB state right before configuring wake sources to avoid
  // stale polarity if the cable changed while we were awake.
#ifdef ARDUINO_ESP32S3_DEV
  pinMode(USB_DET_PIN, INPUT);
  pinMode(MENU_BTN_PIN, INPUT);
  pinMode(BACK_BTN_PIN, INPUT);
  pinMode(UP_BTN_PIN, INPUT);
  pinMode(DOWN_BTN_PIN, INPUT);  
#endif
  display.hibernate();
  RTC.clearAlarm(); // resets the alarm flag in the RTC
#ifdef ARDUINO_ESP32S3_DEV
  esp_sleep_enable_ext0_wakeup((gpio_num_t)USB_DET_PIN, USB_PLUGGED_IN ? LOW : HIGH); //// enable deep sleep wake on USB plug in/out
  rtc_gpio_set_direction((gpio_num_t)USB_DET_PIN, RTC_GPIO_MODE_INPUT_ONLY);
  rtc_gpio_pullup_en((gpio_num_t)USB_DET_PIN);

  esp_sleep_enable_ext1_wakeup(
      BTN_PIN_MASK,
      ESP_EXT1_WAKEUP_ANY_LOW); // enable deep sleep wake on button press
  // Ensure all button pins are configured as RTC inputs with pull-ups
  rtc_gpio_set_direction((gpio_num_t)MENU_BTN_PIN, RTC_GPIO_MODE_INPUT_ONLY);
  rtc_gpio_pullup_en((gpio_num_t)MENU_BTN_PIN);

  rtc_gpio_set_direction((gpio_num_t)BACK_BTN_PIN, RTC_GPIO_MODE_INPUT_ONLY);
  rtc_gpio_pullup_en((gpio_num_t)BACK_BTN_PIN);

  rtc_gpio_set_direction((gpio_num_t)UP_BTN_PIN, RTC_GPIO_MODE_INPUT_ONLY);
  rtc_gpio_pullup_en((gpio_num_t)UP_BTN_PIN);

  rtc_gpio_set_direction((gpio_num_t)DOWN_BTN_PIN, RTC_GPIO_MODE_INPUT_ONLY);
  rtc_gpio_pullup_en((gpio_num_t)DOWN_BTN_PIN);

  rtc_clk_32k_enable(true);

  struct timeval nowTv;
  gettimeofday(&nowTv, nullptr);

  uint64_t secPart = static_cast<uint64_t>(nowTv.tv_sec % 60);
  uint64_t usecPart = static_cast<uint64_t>(nowTv.tv_usec);
  uint64_t usecToNextMin = (60ULL - secPart) * uS_TO_S_FACTOR;
  if (usecPart < usecToNextMin) {
    usecToNextMin -= usecPart;
  } else {
    usecToNextMin = 60ULL * uS_TO_S_FACTOR;
  }

  esp_sleep_enable_timer_wakeup(usecToNextMin); // wake up on the next minute tick to update the display
#else
  // Set GPIOs 0-39 to input to avoid power leaking out
  const uint64_t ignore = 0b11110001000000110000100111000010; // Ignore some GPIOs due to resets
  for (int i = 0; i < GPIO_NUM_MAX; i++)
  {
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

void Watchy::handleButtonPress()
{
  uint64_t wakeupBit = esp_sleep_get_ext1_wakeup_status();
  // Menu Button
  if (wakeupBit & MENU_BTN_MASK)
  {
    if (guiState == WATCHFACE_STATE)
    { // enter menu state if coming from watch face
      returnToTopMenu();
      //menuIndex = 0;
      showMenu(menuIndex);
    }
    else if (guiState == MAIN_MENU_STATE)
    {
      if (!isInSubMenu()) {
        enterSubMenu(menuIndex);
        showMenu(menuIndex);
      } else {
        launchMenuAction(menuCategory, menuIndex);
      }
    }
  }
  // Back Button
  else if (wakeupBit & BACK_BTN_MASK)
  {
    if (guiState == MAIN_MENU_STATE)
    { // exit to watch face if already in menu
      if (isInSubMenu()) {
        returnToTopMenu();
        showMenu(menuIndex);
      } else {
        RTC.read(currentTime);
        showWatchFace(true);
      }
    }
    else if (guiState == APP_STATE || guiState == FW_UPDATE_STATE)
    {
      showMenu(menuIndex); // exit to menu if already in app
    }
  }
  // Up Button
  else if (wakeupBit & UP_BTN_MASK)
  {
    if (guiState == MAIN_MENU_STATE)
    { // increment menu index
      uint8_t len = activeMenuLength();
      if (len > 0)
      {
        menuIndex = (menuIndex == 0) ? static_cast<int>(len - 1) : static_cast<int>(menuIndex - 1);
        showMenu(menuIndex);
      }
    }
  }
  // Down Button
  else if (wakeupBit & DOWN_BTN_MASK)
  {
    if (guiState == MAIN_MENU_STATE)
    { // decrement menu index
      uint8_t len = activeMenuLength();
      if (len > 0)
      {
        menuIndex = (menuIndex + 1 >= len) ? 0 : static_cast<int>(menuIndex + 1);
        showMenu(menuIndex);
      }
    }
  }

  deepSleep();
}

void Watchy::vibMotor(uint8_t intervalMs, uint8_t length)
{
  pinMode(VIB_MOTOR_PIN, OUTPUT);
  bool motorOn = false;
  for (int i = 0; i < length; i++)
  {
    motorOn = !motorOn;
    digitalWrite(VIB_MOTOR_PIN, motorOn);
    delay(intervalMs);
  }
  digitalWrite(VIB_MOTOR_PIN, false);
}

weatherData Watchy::getWeatherData()
{
  return _getWeatherData(settings.cityID, settings.lat, settings.lon,
                         settings.weatherUnit, settings.weatherLang, settings.weatherURL,
                         settings.weatherAPIKey, settings.weatherUpdateInterval);
}

weatherData Watchy::_getWeatherData(String cityID, String lat, String lon, String units, String lang,
                                    String url, String apiKey,
                                    uint8_t updateInterval)
{
  currentWeather.isMetric = units == String("metric");
  if (weatherIntervalCounter < 0)
  { //-1 on first run, set to updateInterval
    weatherIntervalCounter = updateInterval;
  }
  if (weatherIntervalCounter >=
      updateInterval)
  { // only update if WEATHER_UPDATE_INTERVAL has elapsed
    // i.e. 30 minutes
    if (connectWiFi())
    {
      HTTPClient http;              // Use Weather API for live data if WiFi is connected
      http.setConnectTimeout(3000); // 3 second max timeout
      String weatherQueryURL = url;
      if (cityID != "")
      {
        weatherQueryURL.replace("{cityID}", cityID);
      }
      else
      {
        weatherQueryURL.replace("{lat}", lat);
        weatherQueryURL.replace("{lon}", lon);
      }
      weatherQueryURL.replace("{units}", units);
      weatherQueryURL.replace("{lang}", lang);
      weatherQueryURL.replace("{apiKey}", apiKey);
      http.begin(weatherQueryURL.c_str());
      int httpResponseCode = http.GET();
      if (httpResponseCode == 200)
      {
        String payload = http.getString();
        JSONVar responseObject = JSON.parse(payload);
        currentWeather.temperature = int(responseObject["main"]["temp"]);
        currentWeather.weatherConditionCode =
            int(responseObject["weather"][0]["id"]);
        currentWeather.weatherDescription =
            JSONVar::stringify(responseObject["weather"][0]["main"]);
        currentWeather.external = true;
        breakTime((time_t)(int)responseObject["sys"]["sunrise"], currentWeather.sunrise);
        breakTime((time_t)(int)responseObject["sys"]["sunset"], currentWeather.sunset);
      }
      else
      {
        // http error
      }
      http.end();
      // turn off radios
      WiFi.mode(WIFI_OFF);
      btStop();
    }
    else
    {                                                 // No WiFi, use internal temperature sensor
      uint8_t temperature = sensor.readTemperature(); // celsius
      if (!currentWeather.isMetric)
      {
        temperature = temperature * 9. / 5. + 32.; // fahrenheit
      }
      currentWeather.temperature = temperature;
      currentWeather.weatherConditionCode = 800;
      currentWeather.external = false;
    }
    weatherIntervalCounter = 0;
  }
  else
  {
    weatherIntervalCounter++;
  }
  return currentWeather;
}

float Watchy::getBatteryVoltage()
{
#ifdef ARDUINO_ESP32S3_DEV
  return ((analogReadMilliVolts(BATT_ADC_PIN) / 1000.0f * ADC_VOLTAGE_DIVIDER) * 1.0632911f);
#else
  if (RTC.rtcType == DS3231)
  {
    return analogReadMilliVolts(BATT_ADC_PIN) / 1000.0f *
           2.0f; // Battery voltage goes through a 1/2 divider.
  }
  else
  {
    return analogReadMilliVolts(BATT_ADC_PIN) / 1000.0f * 2.0f;
  }
#endif
}

uint8_t Watchy::getBoardRevision()
{
  esp_chip_info_t chip_info;
  esp_chip_info(&chip_info);
  if (chip_info.model == CHIP_ESP32)
  {                               // Revision 1.0 - 2.0
    Wire.beginTransmission(0x68); // v1.0 has DS3231
    if (Wire.endTransmission() == 0)
    {
      return 10;
    }
    delay(1);
    Wire.beginTransmission(0x51); // v1.5 and v2.0 have PCF8563
    if (Wire.endTransmission() == 0)
    {
      pinMode(35, INPUT);
      if (digitalRead(35) == 0)
      {
        return 20; // in rev 2.0, pin 35 is BTN 3 and has a pulldown
      }
      else
      {
        return 15; // in rev 1.5, pin 35 is the battery ADC
      }
    }
  }
  if (chip_info.model == CHIP_ESP32S3)
  { // Revision 3.0
    return 30;
  }
  return -1;
}

uint16_t Watchy::_readRegister(uint8_t address, uint8_t reg, uint8_t *data,
                               uint16_t len)
{
  Wire.beginTransmission(address);
  Wire.write(reg);
  Wire.endTransmission();
  Wire.requestFrom((uint8_t)address, (uint8_t)len);
  uint8_t i = 0;
  while (Wire.available())
  {
    data[i++] = Wire.read();
  }
  return 0;
}

uint16_t Watchy::_writeRegister(uint8_t address, uint8_t reg, uint8_t *data,
                                uint16_t len)
{
  Wire.beginTransmission(address);
  Wire.write(reg);
  Wire.write(data, len);
  return (0 != Wire.endTransmission());
}

void Watchy::_bmaConfig()
{

  if (sensor.begin(_readRegister, _writeRegister, delay) == false)
  {
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
  config.lvl = BMA4_ACTIVE_HIGH;
  config.od = BMA4_PUSH_PULL;
  config.output_en = BMA4_OUTPUT_ENABLE;
  config.input_en = BMA4_INPUT_DISABLE;
  // The correct trigger interrupt needs to be configured as needed
  sensor.setINTPinConfig(config, BMA4_INTR1_MAP);

  struct bma423_axes_remap remap_data;
  remap_data.x_axis = 1;
  remap_data.x_axis_sign = 0xFF;
  remap_data.y_axis = 0;
  remap_data.y_axis_sign = 0xFF;
  remap_data.z_axis = 2;
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

void Watchy::_configModeCallback(WiFiManager *myWiFiManager)
{
  display.setFullWindow();
  display.clearScreen(UiSDK::getWatchfaceBg(BASE_POLARITY));
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(UiSDK::getWatchfaceFg(BASE_POLARITY));
  display.setCursor(0, 30);
  display.println("Connect to");
  display.print("SSID: ");
  display.println(WIFI_AP_SSID);
  display.print("IP: ");
  display.println(WiFi.softAPIP());
  display.println("MAC address:");
  display.println(WiFi.softAPmacAddress().c_str());
  display.display(true); // partial refresh
}


