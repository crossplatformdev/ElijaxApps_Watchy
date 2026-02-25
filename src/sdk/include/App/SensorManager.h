#ifndef WATCHY_SENSOR_MANAGER_H
#define WATCHY_SENSOR_MANAGER_H

#include <Arduino.h>

struct bma4_accel;

namespace WatchySensor {

struct Diagnostics {
  bool accelerationEnabled;
  uint8_t status;
  uint8_t error;
};

enum class Mode : uint8_t {
  None,
  Baseline,
  FallMonitoring,
  WatchfaceBcg,
  ForegroundHeartRate,
  ForegroundFall,
  LiveAcceleration
};

bool initializeBaseline();
bool setBackgroundMode(Mode mode);
bool reapplyBackgroundMode();
bool acquireForeground(Mode mode);
bool releaseForeground(Mode mode);
Mode backgroundMode();
Mode activeMode();
bool readAccelFifo(bma4_accel *samples, uint16_t capacity,
                   uint16_t &sampleCount);
bool readAcceleration(bma4_accel &acceleration);
uint8_t readDirection();
Diagnostics readDiagnostics();
bool setActivityEnabled(bool enabled);
const char *readActivity();
float readTemperature();
bool readInterruptStatus(uint16_t &interruptStatus);
uint32_t stepCount();
bool resetStepCount();

} // namespace WatchySensor

#endif