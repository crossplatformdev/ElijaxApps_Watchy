//Derived from peerdavid's source at: https://github.com/peerdavid/Watchy
#include "Watchy_Base.h"

RTC_DATA_ATTR int16_t dktime_alarm_timer = -1;
RTC_DATA_ATTR bool dktime_twelve_mode = true;
RTC_DATA_ATTR bool dktime_sleep_mode = false;
RTC_DATA_ATTR bool dktime_playAnim = false;

DkTimeWatchyBase::DkTimeWatchyBase() {}

void DkTimeWatchyBase::init() {
  // This watchface ships with its own base class that relies on a different
  // RTC/sleep API than this firmware. For integration here, we only need
  // drawing, so keep init minimal and compatible.
  RTC.read(currentTime);
  showWatchFace(true);
}

void DkTimeWatchyBase::deepSleep() {
  // No-op for this integrated watchface.
}

void DkTimeWatchyBase::handleButtonPress() {
  uint64_t wakeupBit = esp_sleep_get_ext1_wakeup_status();

  if (IS_DOUBLE_TAP) {
    // Interrupt handling managed by core Watchy

    // To be defined in the watch face what we want exactly
    // to do. Therefore, no return;
  }

  if (IS_BTN_LEFT_UP) {
    dktime_twelve_mode = (dktime_twelve_mode == 0) ? true : false;
    RTC.read(currentTime);
    vibrate();
    showWatchFace(true);
    return;
  }

  if (IS_BTN_RIGHT_UP) {
    RTC.read(currentTime);
    vibrate();
    dktime_playAnim = 1;
    showWatchFace(true);
    return;
  }

  if (IS_BTN_RIGHT_DOWN) {
    RTC.read(currentTime);
    vibrate();
    dktime_playAnim = true;
    showWatchFace(true);
    return;
  }

  Watchy::handleButtonPress();
}

void DkTimeWatchyBase::vibrate(uint8_t times, uint32_t delay_time) {
  pinMode(VIB_MOTOR_PIN, OUTPUT);
  for (uint8_t i = 0; i < times; i++) {
    delay(delay_time);
    digitalWrite(VIB_MOTOR_PIN, true);
    delay(delay_time);
    digitalWrite(VIB_MOTOR_PIN, false);
  }
}

void DkTimeWatchyBase::_rtcConfig() {
  RTC.read(currentTime);
}

uint16_t DkTimeWatchyBase::_readRegister(uint8_t address, uint8_t reg, uint8_t *data, uint16_t len)
{
  Wire.beginTransmission(address);
  Wire.write(reg);
  Wire.endTransmission();
  Wire.requestFrom((uint8_t)address, (uint8_t)len);
  uint8_t i = 0;
  while (Wire.available()) {
    data[i++] = Wire.read();
  }
  return 0;
}

uint16_t DkTimeWatchyBase::_writeRegister(uint8_t address, uint8_t reg, uint8_t *data, uint16_t len)
{
  Wire.beginTransmission(address);
  Wire.write(reg);
  Wire.write(data, len);
  return (0 !=  Wire.endTransmission());
}
