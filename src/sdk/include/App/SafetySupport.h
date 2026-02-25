#ifndef WATCHY_SAFETY_SUPPORT_H
#define WATCHY_SAFETY_SUPPORT_H

#include <WatchySdk.h>

namespace WatchySafetyTools {

String clipped(const String &value, size_t maximum = 18);
float magnitudeG(const Accel &acceleration);

} // namespace WatchySafetyTools

#endif