#include "WatchyUi.h"

#include "AppDisplay.h"
#include "ClockToolApps.h"

namespace WatchyClockTools {

void drawUtcClock(const tmElements_t &time) {
  tmElements_t utc;
  breakTime(currentUtcTime(time), utc);
  char formattedTime[6];
  char date[16];
  char weekday[8];
  WatchyUi::Selector::formatTime(formattedTime, utc.Hour, utc.Minute);
  snprintf(date, sizeof(date), "%04u-%02u-%02u", tmYearToCalendar(utc.Year),
           utc.Month, utc.Day);
  snprintf(weekday, sizeof(weekday), "%u", utc.Wday);
  AppVisual::drawMetric({12, 39, 176, 88}, "UNIVERSAL TIME", formattedTime);
  AppVisual::drawDataRow(150, "DATE", date, true);
  AppVisual::drawDataRow(174, "WEEKDAY", weekday);
}

} // namespace WatchyClockTools
