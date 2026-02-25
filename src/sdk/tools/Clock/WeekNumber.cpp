#include "WatchyUi.h"

#include "AppDisplay.h"
#include "ClockSupport.h"
#include "ClockToolApps.h"

namespace WatchyClockTools {

void drawWeekNumber(const tmElements_t &time) {
  int year = tmYearToCalendar(time.Year);
  int week = ClockSupport::isoWeek(year, time.Month, time.Day);
  char value[6];
  char yearText[8];
  snprintf(value, sizeof(value), "W%02d", week);
  snprintf(yearText, sizeof(yearText), "%d", year);
  AppVisual::drawMetric({12, 38, 176, 92}, "ISO WEEK", value,
                        static_cast<float>(week) / ClockSupport::isoWeeksInYear(year),
                        "Week of the current year");
  AppVisual::drawDataRow(160, "ISO YEAR", yearText, true);
}

} // namespace WatchyClockTools
