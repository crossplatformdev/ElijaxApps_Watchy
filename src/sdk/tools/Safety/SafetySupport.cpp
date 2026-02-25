#include "SafetySupport.h"

#include "WatchyUi.h"

#include <math.h>

namespace WatchySafetyTools {

String clipped(const String &value, size_t maximum) {
  if (value.length() <= maximum) return value;
  return value.substring(0, maximum - 3) + "...";
}

float magnitudeG(const Accel &acceleration) {
  float x = acceleration.x / 256.0f;
  float y = acceleration.y / 256.0f;
  float z = acceleration.z / 256.0f;
  return sqrtf(x * x + y * y + z * z);
}

} // namespace WatchySafetyTools
