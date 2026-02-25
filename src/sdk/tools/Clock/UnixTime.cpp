#include "WatchyUi.h"

#include "AppDisplay.h"
#include "ClockToolApps.h"

namespace WatchyClockTools {

void drawUnixTime(const tmElements_t &time) {
  unsigned long epoch = static_cast<unsigned long>(currentUtcTime(time));
  char value[16];
  char hexadecimal[16];
  snprintf(value, sizeof(value), "%lu", epoch);
  snprintf(hexadecimal, sizeof(hexadecimal), "0x%lX", epoch);
  AppVisual::drawMetric({12, 42, 176, 92}, "UNIX EPOCH", value, -1.0f,
                        "UTC seconds since 1970");
  AppVisual::drawDataRow(162, "HEX", hexadecimal, true);
}

} // namespace WatchyClockTools
