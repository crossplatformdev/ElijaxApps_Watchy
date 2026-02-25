#ifndef WATCHY_CLOCK_SUPPORT_H
#define WATCHY_CLOCK_SUPPORT_H

namespace WatchyClockTools {
namespace ClockSupport {

bool isLeapYear(int year);
int daysInMonth(int year, int month);
int ordinalDay(int year, int month, int day);
int mondayWeekday(int year, int month, int day);
int isoWeeksInYear(int year);
int isoWeek(int year, int month, int day);

} // namespace ClockSupport
} // namespace WatchyClockTools

#endif