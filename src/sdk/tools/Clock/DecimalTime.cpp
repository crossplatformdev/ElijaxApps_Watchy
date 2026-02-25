#include "WatchyUi.h"

#include "AppDisplay.h"
#include "ClockToolApps.h"

namespace WatchyClockTools {

void drawDecimalTime(const tmElements_t &time) {
  unsigned long seconds = time.Hour * SECS_PER_HOUR +
                          time.Minute * SECS_PER_MIN + time.Second;
  unsigned long decimal = seconds * 100000UL / SECS_PER_DAY;
  uint8_t hour = decimal / 10000;
  uint8_t minute = decimal / 100 % 100;
  uint8_t second = decimal % 100;
  char value[10];
  snprintf(value, sizeof(value), "%u:%02u:%02u", hour, minute, second);
  AppVisual::drawMetric({12, 42, 176, 94}, "DECIMAL TIME", value,
                        static_cast<float>(decimal) / 100000.0f,
                        "10 hours, 100 minutes, 100 seconds");
}

} // namespace WatchyClockTools
