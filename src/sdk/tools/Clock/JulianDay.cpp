#include "WatchyUi.h"

#include "AppDisplay.h"
#include "ClockToolApps.h"

namespace WatchyClockTools {

void drawJulianDay(const tmElements_t &time) {
  double julian = static_cast<double>(currentUtcTime(time)) / SECS_PER_DAY +
                  2440587.5;
  char value[16];
  char modified[16];
  snprintf(value, sizeof(value), "%.3f", julian);
  snprintf(modified, sizeof(modified), "%.3f", julian - 2400000.5);
  AppVisual::drawMetric({12, 42, 176, 94}, "ASTRONOMICAL JD", value);
  AppVisual::drawDataRow(166, "MODIFIED JD", modified, true);
}

} // namespace WatchyClockTools
