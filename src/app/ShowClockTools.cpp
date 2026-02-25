#include <Watchy.h>
#include "AppDisplay.h"

namespace {

enum ClockTool : uint8_t {
  BINARY_CLOCK,
  UNIX_TIME,
  UTC_CLOCK,
  WEEK_NUMBER,
  DAY_OF_YEAR,
  MONTH_CALENDAR,
  WORLD_CLOCKS,
  DUAL_TIME,
  INTERNET_BEATS,
  DECIMAL_TIME,
  JULIAN_DAY,
  TIME_PROGRESS,
  CLOCK_TOOL_COUNT
};

const char *const titles[CLOCK_TOOL_COUNT] = {
    "BINARY CLOCK", "UNIX TIME",      "UTC CLOCK",     "ISO WEEK",
    "DAY OF YEAR",  "MONTH CALENDAR", "WORLD CLOCKS",  "LOCAL + UTC",
    "INTERNET BEATS", "DECIMAL TIME", "JULIAN DAY",    "TIME PROGRESS"};

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

void useSmallText(int16_t x, int16_t y) {
  Watchy::display.setFont();
  Watchy::display.setTextSize(1);
  Watchy::display.setCursor(x, y);
}

void printDate(const tmElements_t &value) {
  Watchy::display.print(tmYearToCalendar(value.Year));
  Watchy::display.print('-');
  printTwoDigits(value.Month);
  Watchy::display.print('-');
  printTwoDigits(value.Day);
}

void printClock(const char *label, time_t value, int16_t y) {
  tmElements_t clock;
  breakTime(value, clock);
  Watchy::display.setCursor(4, y);
  Watchy::display.print(label);
  Watchy::display.setCursor(92, y);
  printTwoDigits(clock.Hour);
  Watchy::display.print(':');
  printTwoDigits(clock.Minute);
}

void drawBar(int16_t y, float progress) {
  const uint16_t foreground = DARKMODE ? GxEPD_WHITE : GxEPD_BLACK;
  progress = constrain(progress, 0.0f, 1.0f);
  Watchy::display.drawRect(4, y, 192, 13, foreground);
  Watchy::display.fillRect(7, y + 3, static_cast<int16_t>(186 * progress), 7,
                           foreground);
}

void drawBinary(const tmElements_t &now) {
  const uint8_t values[] = {now.Hour, now.Minute, now.Second};
  const char *const labels[] = {"HOUR", "MIN", "SEC"};
  useSmallText(8, 48);
  for (uint8_t row = 0; row < 3; row++) {
    Watchy::display.setCursor(8, 48 + row * 44);
    Watchy::display.print(labels[row]);
    Watchy::display.setCursor(72, 48 + row * 44);
    for (int bit = 5; bit >= 0; bit--) {
      Watchy::display.print(values[row] & (1U << bit) ? "1 " : "0 ");
    }
    Watchy::display.setCursor(72, 62 + row * 44);
    Watchy::display.print(values[row]);
  }
}

void drawUnix(const tmElements_t &now) {
  unsigned long epoch = static_cast<unsigned long>(currentUtcTime(now));
  useSmallText(8, 52);
  Watchy::display.println("SECONDS SINCE");
  Watchy::display.println("1970-01-01 UTC");
  Watchy::display.setTextSize(2);
  Watchy::display.setCursor(8, 110);
  Watchy::display.println(epoch);
  Watchy::display.setTextSize(1);
  Watchy::display.setCursor(8, 150);
  Watchy::display.print("HEX 0x");
  Watchy::display.print(epoch, HEX);
}

void drawUtc(const tmElements_t &now) {
  tmElements_t utc;
  breakTime(currentUtcTime(now), utc);
  useSmallText(18, 58);
  printDate(utc);
  Watchy::display.setTextSize(3);
  Watchy::display.setCursor(18, 112);
  printTwoDigits(utc.Hour);
  Watchy::display.print(':');
  printTwoDigits(utc.Minute);
  Watchy::display.setTextSize(1);
  Watchy::display.setCursor(18, 150);
  Watchy::display.print("UTC weekday ");
  Watchy::display.print(utc.Wday);
}

void drawWeek(const tmElements_t &now) {
  int year = tmYearToCalendar(now.Year);
  int week = isoWeek(year, now.Month, now.Day);
  useSmallText(12, 55);
  Watchy::display.print("ISO YEAR ");
  Watchy::display.println(year);
  Watchy::display.setTextSize(4);
  Watchy::display.setCursor(58, 120);
  if (week < 10) {
    Watchy::display.print('0');
  }
  Watchy::display.println(week);
  Watchy::display.setTextSize(1);
  Watchy::display.setCursor(35, 160);
  Watchy::display.print(isoWeeksInYear(year));
  Watchy::display.print(" weeks this year");
}

void drawOrdinal(const tmElements_t &now) {
  int year = tmYearToCalendar(now.Year);
  int ordinal = ordinalDay(year, now.Month, now.Day);
  int total = isLeapYear(year) ? 366 : 365;
  useSmallText(22, 52);
  Watchy::display.print("DAY ");
  Watchy::display.print(ordinal);
  Watchy::display.print(" OF ");
  Watchy::display.println(total);
  drawBar(74, static_cast<float>(ordinal) / total);
  Watchy::display.setCursor(22, 115);
  Watchy::display.print(total - ordinal);
  Watchy::display.println(" days remain");
  Watchy::display.setCursor(22, 145);
  Watchy::display.print(isLeapYear(year) ? "LEAP YEAR" : "COMMON YEAR");
}

void drawCalendar(const tmElements_t &now) {
  int year = tmYearToCalendar(now.Year);
  int first = mondayWeekday(year, now.Month, 1) - 1;
  int count = daysInMonth(year, now.Month);
  useSmallText(45, 34);
  Watchy::display.print(monthShortStr(now.Month));
  Watchy::display.print(' ');
  Watchy::display.println(year);
  Watchy::display.setCursor(8, 52);
  Watchy::display.println("MO TU WE TH FR SA SU");
  for (int day = 1; day <= count; day++) {
    int cell = first + day - 1;
    int x = 8 + (cell % 7) * 27;
    int y = 70 + (cell / 7) * 21;
    Watchy::display.setCursor(x, y);
    if (day < 10) {
      Watchy::display.print(' ');
    }
    Watchy::display.print(day);
    if (day == now.Day) {
      WatchyUi::Canvas::outline(
          WatchyUi::Bounds{static_cast<int16_t>(x - 2),
               static_cast<int16_t>(y - 2), 20, 13},
          DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
    }
  }
}

void drawWorld(const tmElements_t &now) {
  time_t utc = currentUtcTime(now);
  useSmallText(4, 42);
  printClock("LOS ANGELES", utc - 8 * SECS_PER_HOUR, 42);
  printClock("NEW YORK", utc - 5 * SECS_PER_HOUR, 68);
  printClock("LONDON", utc, 94);
  printClock("DELHI", utc + 5 * SECS_PER_HOUR + 30 * SECS_PER_MIN, 120);
  printClock("TOKYO", utc + 9 * SECS_PER_HOUR, 146);
  printClock("SYDNEY", utc + 10 * SECS_PER_HOUR, 172);
}

void drawDual(const tmElements_t &now) {
  tmElements_t utc;
  breakTime(currentUtcTime(now), utc);
  useSmallText(10, 52);
  Watchy::display.println("LOCAL");
  Watchy::display.setTextSize(2);
  Watchy::display.setCursor(10, 78);
  printTwoDigits(now.Hour);
  Watchy::display.print(':');
  printTwoDigits(now.Minute);
  Watchy::display.setTextSize(1);
  Watchy::display.setCursor(10, 115);
  Watchy::display.println("UTC");
  Watchy::display.setTextSize(2);
  Watchy::display.setCursor(10, 141);
  printTwoDigits(utc.Hour);
  Watchy::display.print(':');
  printTwoDigits(utc.Minute);
}

void drawBeats(const tmElements_t &now) {
  unsigned long seconds =
      (static_cast<unsigned long>(currentUtcTime(now)) + SECS_PER_HOUR) %
      SECS_PER_DAY;
  float beats = seconds / 86.4f;
  useSmallText(20, 58);
  Watchy::display.println("BIEL MEAN TIME");
  Watchy::display.setTextSize(3);
  Watchy::display.setCursor(25, 115);
  Watchy::display.print('@');
  Watchy::display.println(beats, 2);
  Watchy::display.setTextSize(1);
  Watchy::display.setCursor(20, 155);
  Watchy::display.println("1000 beats per day");
}

void drawDecimal(const tmElements_t &now) {
  unsigned long seconds = now.Hour * SECS_PER_HOUR +
                          now.Minute * SECS_PER_MIN + now.Second;
  unsigned long decimal = seconds * 100000UL / SECS_PER_DAY;
  uint8_t hour = decimal / 10000;
  uint8_t minute = decimal / 100 % 100;
  uint8_t second = decimal % 100;
  useSmallText(15, 55);
  Watchy::display.println("10 HOURS / DAY");
  Watchy::display.setTextSize(2);
  Watchy::display.setCursor(18, 108);
  Watchy::display.print(hour);
  Watchy::display.print(':');
  printTwoDigits(minute);
  Watchy::display.print(':');
  printTwoDigits(second);
  Watchy::display.setTextSize(1);
  Watchy::display.setCursor(15, 150);
  Watchy::display.println("100 min, 100 sec");
}

void drawJulian(const tmElements_t &now) {
  double julian = static_cast<double>(currentUtcTime(now)) / SECS_PER_DAY +
                  2440587.5;
  useSmallText(8, 54);
  Watchy::display.println("ASTRONOMICAL JD");
  Watchy::display.setTextSize(2);
  Watchy::display.setCursor(8, 105);
  Watchy::display.println(julian, 3);
  Watchy::display.setTextSize(1);
  Watchy::display.setCursor(8, 145);
  Watchy::display.print("MJD ");
  Watchy::display.println(julian - 2400000.5, 3);
}

void drawProgress(const tmElements_t &now) {
  int year = tmYearToCalendar(now.Year);
  float minute = now.Second / 60.0f;
  float day = (now.Hour * 3600.0f + now.Minute * 60.0f + now.Second) /
              SECS_PER_DAY;
  float yearProgress =
      (ordinalDay(year, now.Month, now.Day) - 1 + day) /
      (isLeapYear(year) ? 366.0f : 365.0f);
  useSmallText(4, 42);
  Watchy::display.println("MINUTE");
  drawBar(49, minute);
  Watchy::display.setCursor(4, 86);
  Watchy::display.println("DAY");
  drawBar(93, day);
  Watchy::display.setCursor(4, 130);
  Watchy::display.println("YEAR");
  drawBar(137, yearProgress);
  Watchy::display.setCursor(4, 177);
  Watchy::display.print(yearProgress * 100.0f, 1);
  Watchy::display.print("% complete");
}

} // namespace

void Watchy::showClockTool(uint8_t tool) {
  if (tool >= CLOCK_TOOL_COUNT) {
    tool = BINARY_CLOCK;
  }
  RTC.read(currentTime);
  beginAppDisplay(titles[tool]);

  switch (tool) {
  case BINARY_CLOCK: drawBinary(currentTime); break;
  case UNIX_TIME: drawUnix(currentTime); break;
  case UTC_CLOCK: drawUtc(currentTime); break;
  case WEEK_NUMBER: drawWeek(currentTime); break;
  case DAY_OF_YEAR: drawOrdinal(currentTime); break;
  case MONTH_CALENDAR: drawCalendar(currentTime); break;
  case WORLD_CLOCKS: drawWorld(currentTime); break;
  case DUAL_TIME: drawDual(currentTime); break;
  case INTERNET_BEATS: drawBeats(currentTime); break;
  case DECIMAL_TIME: drawDecimal(currentTime); break;
  case JULIAN_DAY: drawJulian(currentTime); break;
  case TIME_PROGRESS: drawProgress(currentTime); break;
  default: break;
  }

  finishAppDisplay();
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
namespace WatchyDemo {

void renderClockPreview(uint8_t tool, const tmElements_t &fixedTime) {
  if (tool >= CLOCK_TOOL_COUNT) {
    tool = BINARY_CLOCK;
  }

  long previousOffset = gmtOffset;
  gmtOffset = 2 * SECS_PER_HOUR;
  beginAppDisplay(titles[tool]);
  switch (tool) {
  case BINARY_CLOCK: drawBinary(fixedTime); break;
  case UNIX_TIME: drawUnix(fixedTime); break;
  case UTC_CLOCK: drawUtc(fixedTime); break;
  case WEEK_NUMBER: drawWeek(fixedTime); break;
  case DAY_OF_YEAR: drawOrdinal(fixedTime); break;
  case MONTH_CALENDAR: drawCalendar(fixedTime); break;
  case WORLD_CLOCKS: drawWorld(fixedTime); break;
  case DUAL_TIME: drawDual(fixedTime); break;
  case INTERNET_BEATS: drawBeats(fixedTime); break;
  case DECIMAL_TIME: drawDecimal(fixedTime); break;
  case JULIAN_DAY: drawJulian(fixedTime); break;
  case TIME_PROGRESS: drawProgress(fixedTime); break;
  default: break;
  }
  finishAppDisplay();
  gmtOffset = previousOffset;
}

} // namespace WatchyDemo
#endif