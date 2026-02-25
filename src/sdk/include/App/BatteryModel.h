#ifndef BATTERY_MODEL_H
#define BATTERY_MODEL_H

#include <Arduino.h>
#include "config.h"

namespace WatchyBattery {

constexpr float EMPTY_VOLTAGE = WATCHY_BATTERY_EMPTY_VOLTAGE;
constexpr float FULL_VOLTAGE = WATCHY_BATTERY_FULL_VOLTAGE;

struct Estimate {
  float voltage;
  float chargeMah;
  uint8_t percent;
};

constexpr uint8_t percentFromCharge(float chargeMah) {
  return chargeMah <= 0.0f
             ? 0
             : chargeMah >= WATCHY_DEFAULT_BATTERY_CAPACITY_MAH
                   ? 100
                   : static_cast<uint8_t>(
                         chargeMah * 100.0f /
                             WATCHY_DEFAULT_BATTERY_CAPACITY_MAH +
                         0.5f);
}

constexpr float estimatedChargeMah(float voltage) {
  return voltage <= EMPTY_VOLTAGE
             ? 0.0f
             : voltage >= FULL_VOLTAGE
                   ? WATCHY_DEFAULT_BATTERY_CAPACITY_MAH
                   : WATCHY_DEFAULT_BATTERY_CAPACITY_MAH *
                         (voltage - EMPTY_VOLTAGE) /
                         (FULL_VOLTAGE - EMPTY_VOLTAGE);
}

inline Estimate estimate(float voltage) {
  float chargeMah = estimatedChargeMah(voltage);
  return {voltage, chargeMah, percentFromCharge(chargeMah)};
}

static_assert(WATCHY_DEFAULT_BATTERY_CAPACITY_MAH > 0.0f,
              "Battery capacity must be positive");
static_assert(FULL_VOLTAGE > EMPTY_VOLTAGE,
        "Full battery voltage must exceed empty voltage");
static_assert(estimatedChargeMah(FULL_VOLTAGE) ==
          WATCHY_DEFAULT_BATTERY_CAPACITY_MAH,
        "Full battery voltage must map to configured capacity");
static_assert(percentFromCharge(estimatedChargeMah(FULL_VOLTAGE)) == 100,
        "Full battery voltage must report 100 percent");

} // namespace WatchyBattery

#endif