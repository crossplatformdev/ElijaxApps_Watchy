#include "WatchyUi.h"

#include "AppDisplay.h"
#include "ClockToolApps.h"

namespace WatchyClockTools {

void drawDualTime(const tmElements_t &time) {
  tmElements_t utc;
  breakTime(currentUtcTime(time), utc);
  char local[6];
  char universal[6];
  WatchyUi::Selector::formatTime(local, time.Hour, time.Minute);
  WatchyUi::Selector::formatTime(universal, utc.Hour, utc.Minute);
  AppVisual::drawMetric({12, 34, 176, 62}, "LOCAL", local);
  AppVisual::drawMetric({12, 108, 176, 62}, "UTC", universal);
}

} // namespace WatchyClockTools
