#include "WatchyUi.h"

#include "AppDisplay.h"
#include "ClockToolApps.h"

namespace WatchyClockTools {

void drawInternetBeats(const tmElements_t &time) {
  unsigned long seconds =
      (static_cast<unsigned long>(currentUtcTime(time)) + SECS_PER_HOUR) %
      SECS_PER_DAY;
  float beats = seconds / 86.4f;
  char value[10];
  snprintf(value, sizeof(value), "@%06.2f", beats);
  AppVisual::drawMetric({12, 42, 176, 94}, "BIEL MEAN TIME", value,
                        beats / 1000.0f, "1000 beats per day");
  AppVisual::drawDataRow(166, "DAY", "Internet time", true);
}

} // namespace WatchyClockTools
