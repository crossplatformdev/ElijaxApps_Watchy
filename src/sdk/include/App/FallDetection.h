#ifndef FALL_DETECTION_H
#define FALL_DETECTION_H

#include <Arduino.h>

namespace FallDetection {

constexpr uint8_t traceCapacity = 4;

struct Status {
  bool enabled;
  bool armed;
  uint8_t traceCount;
  uint8_t capacity;
};

void initialize();
Status status();
bool setEnabled(bool enabled);
bool clearTraces();
bool handleWake();
uint64_t wakeMask();
void exportTraces(Stream &output);
void serviceSerial();

} // namespace FallDetection

#endif