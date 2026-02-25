#include "WatchyUi.h"
#include "AppDisplay.h"
#include "FallDetection.h"
#include "BcgTraceCapture.h"
#include "HeartRate.h"
#include "HealthcareAlerts.h"
#include "SensorManager.h"
#include "TimeTools.h"
#include "MenuModel.h"
#include "WatchyPowerDiagnostics.h"
#include "WatchyStorage.h"
#include "WatchFaceRegistry.h"

#include "esp32-hal-cpu.h"
#include <type_traits>
#include <config.h>
#include <settings.h>
#ifdef ARDUINO_ESP32S3_DEV
#include <soc/esp32s3/rtc.h>
#endif

namespace {

constexpr uint32_t LOW_POWER_CPU_FREQUENCY_MHZ = 40;
constexpr uint32_t RUNTIME_CPU_FREQUENCY_MHZ = 240;
constexpr uint32_t RADIO_CPU_FREQUENCY_MHZ = 80;
constexpr uint8_t MENU_LAYOUT_VERSION = 1;
constexpr uint32_t retainedClockMagic = 0x434C4B31UL;
constexpr uint32_t persistedClockMagic = 0x434C4B32UL;
constexpr char clockStorageNamespace[] = "watchy-clock";
constexpr char clockStorageKey[] = "ntp-time";

struct PersistedClock {
  uint32_t magic;
  tmElements_t time;
  uint32_t checksum;
};

bool validCalendarTime(const tmElements_t &time) {
  return tmYearToCalendar(time.Year) >= 2020 && time.Month >= 1 &&
         time.Month <= 12 && time.Day >= 1 && time.Day <= 31 &&
         time.Hour < 24 && time.Minute < 60 && time.Second < 60;
}

bool loadPersistedCalendarTime(tmElements_t &time) {
  PersistedClock record{};
  if (!WatchySdk::Storage::read(clockStorageNamespace, clockStorageKey,
                                &record, sizeof(record)) ||
      record.magic != persistedClockMagic ||
      record.checksum != WatchySdk::recordChecksum(
                             &record, offsetof(PersistedClock, checksum)) ||
      !validCalendarTime(record.time)) {
    return false;
  }
  time = record.time;
  return true;
}

void persistCalendarTime(const tmElements_t &time) {
  if (!validCalendarTime(time)) return;
  PersistedClock record{persistedClockMagic, time, 0};
  record.checksum = WatchySdk::recordChecksum(
      &record, offsetof(PersistedClock, checksum));
  WatchySdk::Storage::write(clockStorageNamespace, clockStorageKey, &record,
                            sizeof(record));
}

#ifdef ARDUINO_ESP32S3_DEV
void retainCalendarTime(const tmElements_t &time);
bool restoreCalendarTime();
#endif

void sensorDelay(uint32_t durationMs) {
  (void)WatchyUi::deepSleepDelay(durationMs);
}

void serviceFallWake() {
  if (digitalRead(ACC_INT_2_PIN) != ACTIVE_LOW) {
    return;
  }
  FallDetection::handleWake();
}

struct WakeContext {
  esp_sleep_wakeup_cause_t cause;
  uint64_t ext1Bits;
  bool coldBoot;
};

enum class WakeReason : uint8_t {
  ColdBoot,
  Minute,
  ButtonMenu,
  ButtonBack,
  ButtonUp,
  ButtonDown,
  Sensor,
  Usb,
  Unknown
};

#ifdef ARDUINO_ESP32S3_DEV
constexpr gpio_num_t buttonPins[] = {
    static_cast<gpio_num_t>(MENU_BTN_PIN),
    static_cast<gpio_num_t>(BACK_BTN_PIN),
    static_cast<gpio_num_t>(UP_BTN_PIN),
    static_cast<gpio_num_t>(DOWN_BTN_PIN)};
constexpr gpio_num_t sensorPins[] = {
  static_cast<gpio_num_t>(ACC_INT_1_PIN),
  static_cast<gpio_num_t>(ACC_INT_2_PIN)};
#endif

WakeReason classifyExt1Wake(uint64_t ext1Bits) {
  if (ext1Bits & MENU_BTN_MASK) return WakeReason::ButtonMenu;
  if (ext1Bits & BACK_BTN_MASK) return WakeReason::ButtonBack;
  if (ext1Bits & UP_BTN_MASK) return WakeReason::ButtonUp;
  if (ext1Bits & DOWN_BTN_MASK) return WakeReason::ButtonDown;
  if (ext1Bits & (ACC_INT_MASK | ACC_INT_2_MASK)) {
    return WakeReason::Sensor;
  }
  return WakeReason::Unknown;
}

WakeReason classifyWake(const WakeContext &wake) {
  if (wake.coldBoot) return WakeReason::ColdBoot;
  switch (wake.cause) {
#ifdef ARDUINO_ESP32S3_DEV
  case ESP_SLEEP_WAKEUP_TIMER: return WakeReason::Minute;
  case ESP_SLEEP_WAKEUP_EXT0: return WakeReason::Usb;
#else
  case ESP_SLEEP_WAKEUP_EXT0: return WakeReason::Minute;
#endif
  case ESP_SLEEP_WAKEUP_EXT1: return classifyExt1Wake(wake.ext1Bits);
  default: return WakeReason::Unknown;
  }
}

void prepareRuntimeSensorPins() {
#ifdef ARDUINO_ESP32S3_DEV
  for (gpio_num_t pin : sensorPins) {
    rtc_gpio_deinit(pin);
    pinMode(static_cast<uint8_t>(pin), INPUT);
  }
#endif
}

uint16_t bmaReadRegister(uint8_t address, uint8_t reg, uint8_t *data,
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

uint16_t bmaWriteRegister(uint8_t address, uint8_t reg, uint8_t *data,
                         uint16_t len) {
  Wire.beginTransmission(address);
  Wire.write(reg);
  Wire.write(data, len);
  return (0 != Wire.endTransmission());
}

} // namespace

RTC_DATA_ATTR uint32_t retainedClockState = 0;
RTC_DATA_ATTR tmElements_t retainedClock{};
RTC_DATA_ATTR uint64_t retainedClockRtcUs = 0;
RTC_DATA_ATTR uint32_t stepCounterDay = 0;
RTC_DATA_ATTR uint64_t retainedUptimeRtcUs = 0;

watchySettings settings{
#ifdef CITY_ID
  .cityID = CITY_ID,
  .lat = "",
  .lon = "",
#else
  .cityID = "",
  .lat = LAT,
  .lon = LON,
#endif
  .weatherAPIKey = OPENWEATHERMAP_APIKEY,
  .weatherURL = OPENWEATHERMAP_URL,
  .weatherUnit = TEMP_UNIT,
  .weatherLang = TEMP_LANG,
  .weatherUpdateInterval = WEATHER_UPDATE_INTERVAL,
  .ntpServer = NTP_SERVER,
  .gmtOffset = GMT_OFFSET_SEC,
  .vibrateOClock = true,
};

namespace WatchySdk {
namespace Device {
#ifdef ARDUINO_ESP32S3_DEV
Watchy32KRTC RTC;
#else
WatchyRTC RTC;
#endif
GxEPD2_BW<WatchyDisplay, WatchyDisplay::HEIGHT> display(WatchyDisplay{});
RTC_DATA_ATTR tmElements_t currentTime{};
RTC_DATA_ATTR watchySettings settings{};
} // namespace Device
} // namespace WatchySdk

namespace {

uint32_t stepCounterDayKey(const tmElements_t &time) {
  return static_cast<uint32_t>(tmYearToCalendar(time.Year)) * 372UL +
         static_cast<uint32_t>(time.Month) * 31UL + time.Day;
}

void synchronizeStepCounterDay(const tmElements_t &time) {
  if (!validCalendarTime(time)) return;
  uint32_t currentDay = stepCounterDayKey(time);
  if (stepCounterDay == 0) {
    stepCounterDay = currentDay;
    return;
  }
  if (stepCounterDay != currentDay && WatchySensor::resetStepCount()) {
    stepCounterDay = currentDay;
  }
}

} // namespace

namespace WatchySdk {

uint32_t uptimeSeconds() {
#ifdef ARDUINO_ESP32S3_DEV
  if (retainedUptimeRtcUs == 0) return 0;
  uint64_t elapsedSeconds =
      (esp_rtc_get_time_us() - retainedUptimeRtcUs) / uS_TO_S_FACTOR;
  return elapsedSeconds > UINT32_MAX ? UINT32_MAX
                                     : static_cast<uint32_t>(elapsedSeconds);
#else
  return millis() / 1000UL;
#endif
}

} // namespace WatchySdk

#ifdef ARDUINO_ESP32S3_DEV
namespace {

void retainCalendarTime(const tmElements_t &time) {
  if (!validCalendarTime(time)) return;
  retainedClock = time;
  retainedClockRtcUs = esp_rtc_get_time_us();
  retainedClockState = retainedClockMagic;
}

bool restoreCalendarTime() {
  if (retainedClockState != retainedClockMagic ||
      retainedClockRtcUs == 0 || !validCalendarTime(retainedClock)) {
    return false;
  }
  uint64_t nowUs = esp_rtc_get_time_us();
  uint64_t elapsedUs = nowUs - retainedClockRtcUs;
  breakTime(makeTime(retainedClock) + elapsedUs / uS_TO_S_FACTOR,
            retainedClock);
  retainedClockRtcUs = nowUs;
  return true;
}

} // namespace
#endif

namespace WatchySdk {

bool bmaConfig();
void configModeCallback(WiFiManager *myWiFiManager);
void handleButtonEvent(WatchyUi::Event event);
void handleButtonWake(uint64_t wakeupBits);
void handleColdBoot(String datetime);
void handleMinuteWake();
void handleTimerMenuWake();
void handleUsbWake();
void dispatchWake(const WakeContext &wake, String datetime);
void selectMenuEntry();
void leaveMenuLevel();
void moveMenuSelection(int direction, bool fastRefresh);

void setLowPowerCpuFrequency() {
  setCpuFrequencyMhz(LOW_POWER_CPU_FREQUENCY_MHZ);
}

void setRadioCpuFrequency() {
  setCpuFrequencyMhz(RADIO_CPU_FREQUENCY_MHZ);
}

void ensureDisplayInitialized() {
  display.epd2.initWatchy(false);
}

void showColdBootWatchFace() {
  guiState = WATCHFACE_STATE;
  ensureDisplayInitialized();
  display.epd2.asyncPowerOn();
  WatchyUi::Screen::beginCanvas();
  drawWatchFace();
  WatchyUi::Screen::present(WATCHFACE_STATE, false);
}

void beginBootRuntime() {
#ifdef WATCHY_BCG_TRACE_CAPTURE
  setCpuFrequencyMhz(240);
  Serial.begin(115200);
  uint32_t serialDeadline = millis() + 15000;
  while (!Serial && static_cast<int32_t>(serialDeadline - millis()) > 0) {
    vTaskDelay(pdMS_TO_TICKS(10));
  }
  Serial.println("@WATCHY_BCG_READY 1");
  Serial.flush();
#else
  setCpuFrequencyMhz(RUNTIME_CPU_FREQUENCY_MHZ);
#endif
  WatchyUi::Input::setAuxiliaryWakeSource(
      ACC_INT_2_PIN, ACTIVE_LOW, serviceFallWake);
  WatchyUi::Input::begin();
  prepareRuntimeSensorPins();
  WatchyUi::Theme::load();
}

void normalizeMenuStateAfterBoot() {
  if (menuLayoutVersion != MENU_LAYOUT_VERSION) {
    menuLayoutVersion = MENU_LAYOUT_VERSION;
    menuLevel = MENU_LEVEL_CATEGORIES;
    menuCategory = 0;
    menuIndex = 0;
    categoryMenuIndex = 0;
    for (uint8_t category = 0; category < MENU_CATEGORY_COUNT; category++) {
      submenuMenuIndices[category] = 0;
    }
  }
  if (guiState < WATCHFACE_STATE || guiState > FW_UPDATE_STATE) {
    guiState = WATCHFACE_STATE;
  }
}

WakeContext readWakeContext() {
  WakeContext wake{esp_sleep_get_wakeup_cause(), 0, false};
  wake.coldBoot = wake.cause == ESP_SLEEP_WAKEUP_UNDEFINED;
  if (wake.cause == ESP_SLEEP_WAKEUP_EXT1) {
    wake.ext1Bits = esp_sleep_get_ext1_wakeup_status();
  }
  return wake;
}

void beginWakeDiagnostics(const WakeContext &wake) {
  if (retainedUptimeRtcUs == 0) {
    retainedUptimeRtcUs = esp_rtc_get_time_us();
  }
  WatchyDiagnostics::beginWake(wake.cause, wake.ext1Bits);
}

void initializeBusesAndRtc() {
  #ifdef ARDUINO_ESP32S3_DEV
    Wire.begin(WATCHY_V3_SDA, WATCHY_V3_SCL);     // init i2c
  #else
    Wire.begin(SDA, SCL);                         // init i2c
  #endif
  RTC.init();
}

void restoreRetainedClockAfterWake(const WakeContext &wake) {
#ifdef ARDUINO_ESP32S3_DEV
  if (wake.cause != ESP_SLEEP_WAKEUP_UNDEFINED &&
      restoreCalendarTime()) {
    RTC.set(retainedClock);
  }
#endif
}

void init(String datetime) {
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause(); // get wake up reason
  #ifdef ARDUINO_ESP32S3_DEV
    Wire.begin(WATCHY_V3_SDA, WATCHY_V3_SCL);     // init i2c
  #else
    Wire.begin(SDA, SCL);                         // init i2c
  #endif
  RTC.init();
  // Init the display since is almost sure we will use it
  display.epd2.initWatchy();

  switch (wakeup_reason) {
  #ifdef ARDUINO_ESP32S3_DEV
  case ESP_SLEEP_WAKEUP_TIMER: // RTC Alarm
  #else
  case ESP_SLEEP_WAKEUP_EXT0: // RTC Alarm
  #endif
    RTC.read(currentTime);
    switch (guiState) {
    case WATCHFACE_STATE:
      showWatchFace(true); // partial updates on tick
      if (settings.vibrateOClock) {
        if (currentTime.Minute == 0) {
          // The RTC wakes us up once per minute
          vibMotor(75, 4);
        }
      }
      break;
    case MAIN_MENU_STATE:
      // Return to watchface if in menu for more than one tick
      if (alreadyInMenu) {
        guiState = WATCHFACE_STATE;
        showWatchFace(false);
      } else {
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
    if(guiState == WATCHFACE_STATE){
      RTC.read(currentTime);
      showWatchFace(true);
    }
    break;
  #endif
  default: // reset
    RTC.config(datetime);
    bmaConfig();
    #ifdef ARDUINO_ESP32S3_DEV
    pinMode(USB_DET_PIN, INPUT);
    USB_PLUGGED_IN = (digitalRead(USB_DET_PIN) == 1);
    #endif    
    gmtOffset = settings.gmtOffset;
    RTC.read(currentTime);
    RTC.read(bootTime);
    showWatchFace(false); // full update on reset
    vibMotor(75, 4);
    // For some reason, seems to be enabled on first boot
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
    break;
  }
  deepSleep();
}

void handleMinuteWake() {
  RTC.read(currentTime);
  synchronizeStepCounterDay(currentTime);
  FallDetection::handleWake();
  checkDailyAlarm(currentTime);
  HealthcareAlerts::check(currentTime);

  bool heartRateRefresh = serviceWatchfaceHeartRateMonitoring();  
}

void handleButtonWake(uint64_t wakeupBits) {
  uint64_t sensorWakeBits = ACC_INT_MASK | ACC_INT_2_MASK;
  bool heartRateWake = (wakeupBits & watchfaceHeartRateWakeMask()) != 0 &&
                       isWatchfaceHeartRateMonitoringActive();
  if ((wakeupBits & sensorWakeBits) && !heartRateWake) {
    FallDetection::handleWake();
  }  
  if (heartRateWake && serviceWatchfaceHeartRateMonitoring() &&
      guiState == WATCHFACE_STATE) {
    RTC.read(currentTime);
    synchronizeStepCounterDay(currentTime);
    if (!refreshWatchFaceHeartRate()) {
      showWatchFace(false);
    }
  }
  handleButtonPress();
}

void handleUsbWake() {
#ifdef ARDUINO_ESP32S3_DEV
  WatchyUi::Power::usbPluggedIn();
  if (guiState == WATCHFACE_STATE) {
    RTC.read(currentTime);
    synchronizeStepCounterDay(currentTime);
    showWatchFace();
  }
#endif
}

void restoreClockForColdBoot(String datetime) {
  if (retainedClockState == retainedClockMagic &&
      validCalendarTime(retainedClock)) {
#ifdef ARDUINO_ESP32S3_DEV
    restoreCalendarTime();
#endif
    RTC.set(retainedClock);
  } else if (loadPersistedCalendarTime(currentTime)) {
    RTC.set(currentTime);
#ifdef ARDUINO_ESP32S3_DEV
    retainCalendarTime(currentTime);
#endif
  } else {
    RTC.config(datetime);
  }
}

void handleTraceCaptureBoot() {
#ifdef WATCHY_BCG_TRACE_CAPTURE
  constexpr uint32_t traceSerialGracePeriodMs = 5000;
  WatchyBcgTrace::beginAutomatic(40000);
  showHeartRate();
  Serial.println("@WATCHY_BCG_ERROR 1 measurement-ended");
  Serial.flush();
  uint32_t serialDeadline = millis() + traceSerialGracePeriodMs;
  while (static_cast<int32_t>(serialDeadline - millis()) > 0) {
    WatchyBcgTrace::serviceSerial();
    vTaskDelay(pdMS_TO_TICKS(100));
  }
#endif
}

void handleColdBoot(String datetime) {
  restoreClockForColdBoot(datetime);
#ifdef ARDUINO_ESP32S3_DEV
  WatchyUi::Power::usbPluggedIn();
#endif
  gmtOffset = settings.gmtOffset;
  RTC.read(currentTime);
  synchronizeStepCounterDay(currentTime);
  RTC.read(bootTime);
  showColdBootWatchFace();
  vibMotor(75, 4);
  bool sensorReady = bmaConfig();
  if (sensorReady) {
    FallDetection::initialize();
  }
  setWatchfaceHeartRateMonitoring(getSelectedWatchface() == WATCHFACE_7_SEG);
  handleTraceCaptureBoot();
  esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
}

void stopRuntimeForDeepSleep() {
  setLowPowerCpuFrequency();
  stopHeartRateMeasurement();
  if (!waitForHeartRateMeasurement(3000)) {
    WatchyDiagnostics::recordWorkerStopTimeout();
    abortHeartRateMeasurement();
  }
  if (guiState != WATCHFACE_STATE) {
    setWatchfaceHeartRateMonitoring(false);
  }
  WiFi.disconnect(true, false);
  btStop();
  BLE_CONFIGURED = false;
}

void servicePendingWatchfaceWakeBeforeDeepSleep() {
  #ifdef ARDUINO_ESP32S3_DEV
  if (isWatchfaceHeartRateMonitoringActive() &&
      gpio_get_level((gpio_num_t)ACC_INT_2_PIN) == ACTIVE_LOW &&
      serviceWatchfaceHeartRateMonitoring() &&
      guiState == WATCHFACE_STATE) {
    RTC.read(currentTime);
    if (!refreshWatchFaceHeartRate()) {
      showWatchFace(false);
    }
  }
  #endif
}

bool waitForDisplayIdleBeforeDeepSleep() {
  if (!display.epd2.initializedThisWake()) return false;
  uint32_t deadline = millis() + WatchyDisplay::full_refresh_time +
                      WatchyDisplay::power_on_time + 250;
  uint32_t now = millis();
  uint32_t remaining = static_cast<int32_t>(deadline - now) > 0
                           ? deadline - now
                           : 0;
  return WatchyUi::Power::waitForDisplayReady(remaining);
}

void hibernateDisplayForDeepSleep() {
  if (waitForDisplayIdleBeforeDeepSleep()) {
    display.hibernate();
  }
  WatchyDiagnostics::endWake();
}

#ifdef ARDUINO_ESP32S3_DEV
bool configureRtcWakeInput(gpio_num_t pin) {
  return rtc_gpio_init(pin) == ESP_OK &&
         rtc_gpio_set_direction(pin, RTC_GPIO_MODE_INPUT_ONLY) == ESP_OK &&
         rtc_gpio_pullup_en(pin) == ESP_OK;
}

bool configureUsbWakeForDeepSleep() {
  if (WatchyUi::Power::usbPluggedIn()) return false;
  if (!configureRtcWakeInput(static_cast<gpio_num_t>(USB_DET_PIN))) {
    return false;
  }
  return esp_sleep_enable_ext0_wakeup(static_cast<gpio_num_t>(USB_DET_PIN),
                                      HIGH) == ESP_OK;
}

bool configureButtonWakeForDeepSleep() {
  uint64_t sensorWakeMask = FallDetection::wakeMask() |
                            watchfaceHeartRateWakeMask();
  uint64_t wakeMask = BTN_PIN_MASK | sensorWakeMask;
  for (gpio_num_t pin : buttonPins) {
    if (!configureRtcWakeInput(pin)) return false;
  }
  if (sensorWakeMask & ACC_INT_MASK &&
      !configureRtcWakeInput(static_cast<gpio_num_t>(ACC_INT_1_PIN))) {
    return false;
  }
  if (sensorWakeMask & ACC_INT_2_MASK &&
      !configureRtcWakeInput(static_cast<gpio_num_t>(ACC_INT_2_PIN))) {
    return false;
  }
  return esp_sleep_enable_ext1_wakeup(wakeMask,
                                      ESP_EXT1_WAKEUP_ANY_LOW) == ESP_OK;
}

bool configureMinuteTimerWakeForDeepSleep() {
  rtc_clk_32k_enable(true);
  RTC.read(currentTime);
  uint8_t second = currentTime.Second < 60 ? currentTime.Second : 0;
  uint8_t secondsToNextWake = 60 - second;
  return esp_sleep_enable_timer_wakeup(
             static_cast<uint64_t>(secondsToNextWake) * uS_TO_S_FACTOR) ==
         ESP_OK;
}

bool configureWakeSourcesForDeepSleep() {
  esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
  if (!configureUsbWakeForDeepSleep() ||
      !configureButtonWakeForDeepSleep() ||
      !configureMinuteTimerWakeForDeepSleep()) {
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
    return false;
  }
  return true;
}

void restoreRuntimeWakePins() {
  setCpuFrequencyMhz(RUNTIME_CPU_FREQUENCY_MHZ);
  for (gpio_num_t pin : buttonPins) {
    rtc_gpio_deinit(pin);
    pinMode(static_cast<uint8_t>(pin), INPUT_PULLUP);
  }
  for (gpio_num_t pin : sensorPins) {
    rtc_gpio_deinit(pin);
    pinMode(static_cast<uint8_t>(pin), INPUT);
  }
  rtc_gpio_deinit(static_cast<gpio_num_t>(USB_DET_PIN));
  pinMode(USB_DET_PIN, INPUT);
  WatchyUi::Input::begin();
}
#else
void parkGpiosForDeepSleep() {
  const uint64_t ignore = 0b11110001000000110000100111000010;
  for (int i = 0; i < GPIO_NUM_MAX; i++) {
    if ((ignore >> i) & 0b1) continue;
    pinMode(i, INPUT);
  }
}

void configureWakeSourcesForDeepSleep() {
  parkGpiosForDeepSleep();
  esp_sleep_enable_ext0_wakeup((gpio_num_t)RTC_INT_PIN, 0);
  esp_sleep_enable_ext1_wakeup(BTN_PIN_MASK | FallDetection::wakeMask(),
                               ESP_EXT1_WAKEUP_ANY_HIGH);
}
#endif

void startDeepSleep() {
  esp_deep_sleep_start();
}

#ifdef ARDUINO_ESP32S3_DEV
void serviceUsbRuntime() {
  while (WatchyUi::Power::usbPluggedIn()) {
    WatchyUi::Event event = WatchyUi::Input::waitScheduled(100);
    if (event != WatchyUi::Event::NONE) {
      handleButtonEvent(event);
    }
  }
}
#endif

void deepSleep() {
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

  rtc_clk_32k_enable(true);
  //rtc_clk_slow_freq_set(RTC_SLOW_FREQ_32K_XTAL);
  struct tm timeinfo;
  getLocalTime(&timeinfo);
  int secToNextMin = 60 - timeinfo.tm_sec;
  esp_sleep_enable_timer_wakeup(secToNextMin * uS_TO_S_FACTOR);
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

void handleButtonPress() {
  WatchyUi::Input::begin();
  WatchyUi::Event event = WatchyUi::Input::poll();
  WatchySdk::handleButtonEvent(event);
}

void handleButtonEvent(WatchyUi::Event event) {
  if (event == WatchyUi::Event::NONE) return;
  WatchyUi::Input::waitForRelease(event);
  switch (event)
  {
  case WatchyUi::Event::MENU:
    if (guiState == WATCHFACE_STATE) {
      guiState = MAIN_MENU_STATE;
      menuLevel = MENU_LEVEL_CATEGORIES;
      menuIndex = categoryMenuIndex;
      showMenu(menuIndex, false);
    } else if (guiState == MAIN_MENU_STATE) {
      selectMenuEntry();
    }
    break;
  case WatchyUi::Event::BACK:
    if (guiState == MAIN_MENU_STATE) {
      leaveMenuLevel();
    } else if (guiState == APP_STATE || guiState == FW_UPDATE_STATE) {
      guiState = MAIN_MENU_STATE;
      showMenu(menuIndex, false);
    }
    break;
  case WatchyUi::Event::UP:
    if (guiState != MAIN_MENU_STATE) return;
    moveMenuSelection(-1, false);
    break;
  case WatchyUi::Event::DOWN:
    if (guiState != MAIN_MENU_STATE) return;
    moveMenuSelection(1, false);
    break;
  case WatchyUi::Event::NONE:
    break;
  }
}

void selectMenuEntry() {
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
  WatchyUi::Input::begin();
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

void leaveMenuLevel() {
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

void moveMenuSelection(int direction, bool fastRefresh) {
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

void vibMotor(uint16_t intervalMs, uint8_t length) {
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

weatherData getWeatherData() {
  currentWeather.isMetric = settings.weatherUnit == String("metric");
  if (weatherIntervalCounter < 0) { //-1 on first run, set to updateInterval
    weatherIntervalCounter = settings.weatherUpdateInterval;
  }
  if (weatherIntervalCounter >=
      settings.weatherUpdateInterval) { // only update if the interval elapsed
    weatherIntervalCounter = 0;
    if (connectWiFi()) {
      HTTPClient http; // Use Weather API for live data if WiFi is connected
      http.setConnectTimeout(3000); // 3 second max timeout
      String weatherQueryURL = settings.weatherURL;
      if(settings.cityID != ""){
        weatherQueryURL.replace("{cityID}", settings.cityID);
      }else{
        weatherQueryURL.replace("{lat}", settings.lat);
        weatherQueryURL.replace("{lon}", settings.lon);
      }
      weatherQueryURL.replace("{units}", settings.weatherUnit);
      weatherQueryURL.replace("{lang}", settings.weatherLang);
      weatherQueryURL.replace("{apiKey}", settings.weatherAPIKey);
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

weatherData getCachedWeatherData() {
  return currentWeather;
}

float getBatteryVoltage() {
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

uint8_t getBoardRevision() {
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

bool bmaConfig() {
  if (sensor.begin(bmaReadRegister, bmaWriteRegister, sensorDelay) == false) {
    return false;
  }

  struct bma423_axes_remap remap_data;
  remap_data.x_axis      = 1;
  remap_data.x_axis_sign = 0xFF;
  remap_data.y_axis      = 0;
  remap_data.y_axis_sign = 0xFF;
  remap_data.z_axis      = 2;
  remap_data.z_axis_sign = 0xFF;
  // Need to raise the wrist function, need to set the correct axis
  if (!sensor.setRemapAxes(&remap_data) ||
      !WatchySensor::initializeBaseline()) {
    return false;
  }
  return true;
}

void configModeCallback(WiFiManager *myWiFiManager) {
  (void)myWiFiManager;
  WatchyUi::Screen::begin("WI-FI SETUP");
  AppVisual::drawStatusIcon({79, 38, 42, 42}, AppVisual::StatusIcon::RADIO,
                            true);
  WatchyUi::Canvas::centeredText({0, 91, 200, 18}, "JOIN WATCHY AP", 2,
                                 WatchyUi::Theme::foreground());
  AppVisual::drawDataRow(130, "NETWORK", WIFI_AP_SSID, true);
  AppVisual::drawDataRow(154, "BROWSER", "192.168.4.1");
  AppVisual::drawDataRow(178, "SECURITY", "Captive setup portal");
  WatchyUi::Widget::footer("BACK EXIT");
  WatchyUi::Screen::present(APP_STATE);
}

bool syncNTP() { // NTP sync - call after connecting to WiFi and
                              // remember to turn it back off
  return syncNTP(gmtOffset, settings.ntpServer.c_str());
}

bool syncNTP(long gmt) {
  return syncNTP(gmt, settings.ntpServer.c_str());
}

bool syncNTP(long gmt, String ntpServer) {
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
  persistCalendarTime(tm);
  #ifdef ARDUINO_ESP32S3_DEV
  retainCalendarTime(tm);
  #endif
  return true;
}

} // namespace WatchySdk
