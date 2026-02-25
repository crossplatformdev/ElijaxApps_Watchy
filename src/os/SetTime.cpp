#include <Watchy.h>
#include <config.h>
#include "sdk/WatchyUi.h"

extern RTC_DATA_ATTR long gmtOffset;

namespace {

uint8_t daysInMonth(uint8_t month, uint8_t year) {
  constexpr uint8_t days[] = {31, 28, 31, 30, 31, 30,
                              31, 31, 30, 31, 30, 31};
  if (month == 2 && year % 4 == 0) return 29;
  uint8_t validMonth = month < 1 ? 1 : month > 12 ? 12 : month;
  return days[validMonth - 1];
}

void clampDay(uint8_t &day, uint8_t month, uint8_t year) {
  uint8_t maximum = daysInMonth(month, year);
  if (day < 1) day = 1;
  else if (day > maximum) day = maximum;
}

} // namespace

void Watchy::setTime() {

  guiState = APP_STATE;

  RTC.read(currentTime);

  #ifdef ARDUINO_ESP32S3_DEV
  uint8_t minute = currentTime.Minute;
  uint8_t hour   = currentTime.Hour;
  uint8_t day    = currentTime.Day;
  uint8_t month  = currentTime.Month;
  uint8_t year   = currentTime.Year;  
  #else
  int8_t minute = currentTime.Minute;
  int8_t hour   = currentTime.Hour;
  int8_t day    = currentTime.Day;
  int8_t month  = currentTime.Month;
  int8_t year   = tmYearToY2k(currentTime.Year);
  #endif

  long editedGmtOffset = gmtOffset;

  int8_t setIndex = SET_HOUR;

  int8_t blink = 0;

  WatchyUi::Input::begin();
  WatchyUi::Event event = WatchyUi::Event::NONE;

  display.setFullWindow();
  display.fillScreen(DARKMODE ? GxEPD_BLACK : GxEPD_WHITE);
  display.setTextColor(DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);;

  while (true) {

    if (event == WatchyUi::Event::SELECT) {
      setIndex++;
      if (setIndex > SET_GMT_OFFSET) {
        break;
      }
    }
    if (event == WatchyUi::Event::BACK) {
      if (setIndex != SET_HOUR) {
        setIndex--;
      }
    }

    blink = 1 - blink;

    if (event == WatchyUi::Event::DOWN) {
      blink = 1;
      switch (setIndex) {
      case SET_HOUR:
        hour == 23 ? (hour = 0) : hour++;
        break;
      case SET_MINUTE:
        minute == 59 ? (minute = 0) : minute++;
        break;
      case SET_YEAR:
        year == 99 ? (year = 0) : year++;
        clampDay(day, month, year);
        break;
      case SET_MONTH:
        month == 12 ? (month = 1) : month++;
        clampDay(day, month, year);
        break;
      case SET_DAY:
        day == daysInMonth(month, year) ? (day = 1) : day++;
        break;
      case SET_GMT_OFFSET:
        editedGmtOffset = editedGmtOffset >= 14 * SECS_PER_HOUR
            ? -12 * SECS_PER_HOUR
            : editedGmtOffset + 15 * SECS_PER_MIN;
        break;
      default:
        break;
      }
    }

    if (event == WatchyUi::Event::UP) {
      blink = 1;
      switch (setIndex) {
      case SET_HOUR:
        hour == 0 ? (hour = 23) : hour--;
        break;
      case SET_MINUTE:
        minute == 0 ? (minute = 59) : minute--;
        break;
      case SET_YEAR:
        year == 0 ? (year = 99) : year--;
        clampDay(day, month, year);
        break;
      case SET_MONTH:
        month == 1 ? (month = 12) : month--;
        clampDay(day, month, year);
        break;
      case SET_DAY:
        day == 1 ? (day = daysInMonth(month, year)) : day--;
        break;
      case SET_GMT_OFFSET:
        editedGmtOffset = editedGmtOffset <= -12 * SECS_PER_HOUR
            ? 14 * SECS_PER_HOUR
            : editedGmtOffset - 15 * SECS_PER_MIN;
        break;
      default:
        break;
      }
    }

    display.fillScreen(DARKMODE ? GxEPD_BLACK : GxEPD_WHITE);

    display.setTextColor(DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);;
    display.setFont(&DSEG7_Classic_Bold_53);

    display.setCursor(5, 80);
    if (setIndex == SET_HOUR) { // blink hour digits
      display.setTextColor(blink ? WatchyUi::Theme::foreground()
                                 : WatchyUi::Theme::background());
    }
    if (hour < 10) {
      display.print("0");
    }
    display.print(hour);

    display.setTextColor(DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);;
    display.print(":");

    display.setCursor(108, 80);
    if (setIndex == SET_MINUTE) { // blink minute digits
      display.setTextColor(blink ? WatchyUi::Theme::foreground()
                                 : WatchyUi::Theme::background());
    }
    if (minute < 10) {
      display.print("0");
    }
    display.print(minute);

    display.setTextColor(DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);;

    display.setFont(&FreeMonoBold9pt7b);
    display.setCursor(45, 150);
    if (setIndex == SET_YEAR) { // blink minute digits
      display.setTextColor(blink ? WatchyUi::Theme::foreground()
                                 : WatchyUi::Theme::background());
    }
    display.print(2000 + year);

    display.setTextColor(DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);;
    display.print("/");

    if (setIndex == SET_MONTH) { // blink minute digits
      display.setTextColor(blink ? WatchyUi::Theme::foreground()
                                 : WatchyUi::Theme::background());
    }
    if (month < 10) {
      display.print("0");
    }
    display.print(month);

    display.setTextColor(DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);;
    display.print("/");

    if (setIndex == SET_DAY) { // blink minute digits
      display.setTextColor(blink ? WatchyUi::Theme::foreground()
                                 : WatchyUi::Theme::background());
    }
    if (day < 10) {
      display.print("0");
    }
    display.print(day);

    display.setTextColor(DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
    display.setCursor(45, 180);
    if (setIndex == SET_GMT_OFFSET) {
      display.setTextColor(blink ? WatchyUi::Theme::foreground()
                                 : WatchyUi::Theme::background());
    }
    display.print("GMT ");
    display.print(editedGmtOffset < 0 ? '-' : '+');
    long absoluteOffset = editedGmtOffset < 0
        ? -editedGmtOffset
        : editedGmtOffset;
    int offsetHours = absoluteOffset / SECS_PER_HOUR;
    int offsetMinutes = (absoluteOffset % SECS_PER_HOUR) / SECS_PER_MIN;
    if (offsetHours < 10) {
      display.print('0');
    }
    display.print(offsetHours);
    display.print(':');
    if (offsetMinutes < 10) {
      display.print('0');
    }
    display.print(offsetMinutes);
    display.display(true); // partial refresh
    event = WatchyUi::Input::wait(700);
  }

  tmElements_t tm;
  tm.Month  = month;
  tm.Day    = day;
  #ifdef ARDUINO_ESP32S3_DEV
  tm.Year   = year;
  #else
  tm.Year   = y2kYearToTm(year);
  #endif
  tm.Hour   = hour;
  tm.Minute = minute;
  tm.Second = 0;

  RTC.set(tm);
  gmtOffset = editedGmtOffset;

  showMenu(menuIndex, false);
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
namespace WatchyDemo {

void renderSetTimePreview() {
  Watchy::display.setFullWindow();
  Watchy::display.fillScreen(WatchyUi::Theme::background());
  Watchy::display.setTextColor(WatchyUi::Theme::foreground());
  Watchy::display.setFont(&DSEG7_Classic_Bold_53);
  Watchy::display.setCursor(5, 80);
  Watchy::display.print("10:");
  Watchy::display.setCursor(108, 80);
  Watchy::display.print("34");
  Watchy::display.setFont(&FreeMonoBold9pt7b);
  Watchy::display.setCursor(45, 150);
  Watchy::display.print("2026/08/23");
  Watchy::display.setCursor(45, 180);
  Watchy::display.print("GMT +02:00");
  Watchy::display.display(true);
}

} // namespace WatchyDemo
#endif