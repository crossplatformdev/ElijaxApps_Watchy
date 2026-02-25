#include "ClockSupport.h"

#include "WatchyUi.h"

#include <stdint.h>

namespace WatchyClockTools {
namespace ClockSupport {

bool isLeapYear(int year) {
  return year % 4 == 0 && (year % 100 != 0 || year % 400 == 0);
}

int daysInMonth(int year, int month) {
  static const uint8_t days[] = {31, 28, 31, 30, 31, 30,
                                 31, 31, 30, 31, 30, 31};
  return month == 2 && isLeapYear(year) ? 29 : days[month - 1];
}

int ordinalDay(int year, int month, int day) {
  int ordinal = day;
  for (int value = 1; value < month; value++) {
    ordinal += daysInMonth(year, value);
  }
  return ordinal;
}

int mondayWeekday(int year, int month, int day) {
  static const int offsets[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
  if (month < 3) {
    year--;
  }
  int sundayBased =
      (year + year / 4 - year / 100 + year / 400 + offsets[month - 1] + day) % 7;
  return sundayBased == 0 ? 7 : sundayBased;
}

int isoWeeksInYear(int year) {
  int januaryFirst = mondayWeekday(year, 1, 1);
  return januaryFirst == 4 || (januaryFirst == 3 && isLeapYear(year)) ? 53 : 52;
}

int isoWeek(int year, int month, int day) {
  int week = (ordinalDay(year, month, day) - mondayWeekday(year, month, day) + 10) / 7;
  if (week < 1) {
    return isoWeeksInYear(year - 1);
  }
  if (week > isoWeeksInYear(year)) {
    return 1;
  }
  return week;
}

} // namespace ClockSupport
} // namespace WatchyClockTools
