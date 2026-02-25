#include "WatchySdk.h"
#include "WatchyUi.h"
#include "Watchy.h"

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

WatchyUi::Bounds setTimeSelectionBounds(int8_t setIndex) {
  switch (setIndex) {
  case SET_HOUR: return {1, 20, 96, 65};
  case SET_MINUTE: return {104, 20, 95, 65};
  case SET_YEAR: return {42, 130, 49, 25};
  case SET_MONTH: return {96, 130, 29, 25};
  case SET_DAY: return {128, 130, 31, 25};
  case SET_GMT_OFFSET: return {42, 158, 124, 25};
  default: return {0, 0, 0, 0};
  }
}

} // namespace

void setTimeImpl(Watchy *watchy) {

  guiState = APP_STATE;

  tmElements_t currentTime;
  if (watchy != nullptr) {
    watchy->RTC.read(watchy->currentTime);
    currentTime = watchy->currentTime;
  } else {
    WatchySdk::RTC.read(WatchySdk::currentTime);
    currentTime = WatchySdk::currentTime;
  }

  #ifdef ARDUINO_ESP32S3_DEV
  uint8_t minute = currentTime.Minute;
  uint8_t hour   = currentTime.Hour;
  uint8_t day    = currentTime.Day;
  uint8_t month  = currentTime.Month;
  uint8_t year   = tmYearToY2k(currentTime.Year);  
  #else
  int8_t minute = currentTime.Minute;
  int8_t hour   = currentTime.Hour;
  int8_t day    = currentTime.Day;
  int8_t month  = currentTime.Month;
  int8_t year   = tmYearToY2k(currentTime.Year);
  #endif

  long editedGmtOffset = gmtOffset;

  int8_t setIndex = SET_HOUR;

  WatchyUi::Input::begin();
  WatchyUi::Event event = WatchyUi::Event::NONE;

  while (true) {

    if (event == WatchyUi::Event::MENU) {
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

    if (event == WatchyUi::Event::DOWN) {
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

    WatchyUi::Screen::beginCanvas();
    Watchy::display.setTextColor(WatchyUi::Theme::foreground());
    Watchy::display.setFont(&DSEG7_Classic_Bold_53);

    Watchy::display.setCursor(5, 80);
    if (hour < 10) {
      Watchy::display.print("0");
    }
    Watchy::display.print(hour);

    Watchy::display.print(":");

    Watchy::display.setCursor(108, 80);
    if (minute < 10) {
      Watchy::display.print("0");
    }
    Watchy::display.print(minute);

    Watchy::display.setFont(&FreeMonoBold9pt7b);
    Watchy::display.setCursor(45, 150);
    Watchy::display.print(2000 + year);

    Watchy::display.print("/");

    if (month < 10) {
      Watchy::display.print("0");
    }
    Watchy::display.print(month);

    Watchy::display.print("/");

    if (day < 10) {
      Watchy::display.print("0");
    }
    Watchy::display.print(day);

    Watchy::display.setCursor(45, 180);
    Watchy::display.print("GMT ");
    Watchy::display.print(editedGmtOffset < 0 ? '-' : '+');
    long absoluteOffset = editedGmtOffset < 0
        ? -editedGmtOffset
        : editedGmtOffset;
    int offsetHours = absoluteOffset / SECS_PER_HOUR;
    int offsetMinutes = (absoluteOffset % SECS_PER_HOUR) / SECS_PER_MIN;
    if (offsetHours < 10) {
      Watchy::display.print('0');
    }
    Watchy::display.print(offsetHours);
    Watchy::display.print(':');
    if (offsetMinutes < 10) {
      Watchy::display.print('0');
    }
    Watchy::display.print(offsetMinutes);
    WatchyUi::Canvas::outline(setTimeSelectionBounds(setIndex),
                  WatchyUi::Theme::foreground());
    WatchyUi::Screen::present(APP_STATE);
    event = WatchyUi::Input::wait();
  }

  tmElements_t tm;
  tm.Month  = month;
  tm.Day    = day;
  #ifdef ARDUINO_ESP32S3_DEV
  tm.Year   = y2kYearToTm(year);
  #else
  tm.Year   = y2kYearToTm(year);
  #endif
  tm.Hour   = hour;
  tm.Minute = minute;
  tm.Second = 0;

  if (watchy != nullptr) {
    watchy->RTC.set(tm);
    watchy->settings.gmtOffset = editedGmtOffset;
  } else {
    WatchySdk::RTC.set(tm);
  }
  gmtOffset = editedGmtOffset;

  if (watchy != nullptr) {
    watchy->showMenu(menuIndex, false);
  } else {
    WatchySdk::showMenu(menuIndex, false);
  }
}

void Watchy::setTime() { setTimeImpl(this); }

void WatchySdk::setTime() { setTimeImpl(nullptr); }

#ifdef WATCHY_DETERMINISTIC_GALLERY
namespace WatchyDemo {

void renderSetTimePreview(uint8_t view) {
  (void)view;
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
  WatchyUi::Canvas::outline(setTimeSelectionBounds(SET_HOUR),
                            WatchyUi::Theme::foreground());
  Watchy::display.display(true);
}

} // namespace WatchyDemo
#endif
