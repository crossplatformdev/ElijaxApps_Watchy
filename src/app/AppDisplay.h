#ifndef APP_DISPLAY_H
#define APP_DISPLAY_H

#include <Watchy.h>
#include "sdk/WatchyUi.h"

extern RTC_DATA_ATTR long gmtOffset;

inline void beginAppDisplay(const char *title) {
  WatchyUi::Screen::begin(title);
}

inline void finishAppDisplay() {
  WatchyUi::Screen::present();
}

inline void printTwoDigits(uint8_t value) {
  if (value < 10) {
    Watchy::display.print('0');
  }
  Watchy::display.print(value);
}

inline void printDateTime(time_t utcTime, long offset = 0) {
  tmElements_t value;
  breakTime(utcTime + offset, value);
  printTwoDigits(value.Day);
  Watchy::display.print('/');
  printTwoDigits(value.Month);
  Watchy::display.print(' ');
  printTwoDigits(value.Hour);
  Watchy::display.print(':');
  printTwoDigits(value.Minute);
}

inline time_t currentUtcTime(tmElements_t currentTime) {
  return makeTime(currentTime) - gmtOffset;
}

#endif