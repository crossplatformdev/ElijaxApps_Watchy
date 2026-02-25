#include "WatchyUi.h"

#include "AppDisplay.h"
#include "ClockSupport.h"
#include "ClockToolApps.h"

namespace WatchyClockTools {

void drawDayOfYear(const tmElements_t &time) {
  int year = tmYearToCalendar(time.Year);
  int ordinal = ClockSupport::ordinalDay(year, time.Month, time.Day);
  int total = ClockSupport::isLeapYear(year) ? 366 : 365;
  char value[8];
  char remain[18];
  snprintf(value, sizeof(value), "%d", ordinal);
  snprintf(remain, sizeof(remain), "%d days", total - ordinal);
  AppVisual::drawMetric({12, 38, 176, 92}, "DAY OF YEAR", value,
                        static_cast<float>(ordinal) / total,
                        ClockSupport::isLeapYear(year) ? "Leap year" : "Common year");
  AppVisual::drawDataRow(160, "REMAINING", remain, true);
}

} // namespace WatchyClockTools
