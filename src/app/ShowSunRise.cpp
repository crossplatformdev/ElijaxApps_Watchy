#include <SunRise.h>
#include "AppDisplay.h"

#include "WatchyUi.h"
#include "Watchy.h"

namespace {

uint16_t localMinutes(time_t value, long utcOffset) {
  tmElements_t local{};
  breakTime(value + utcOffset, local);
  return local.Hour * 60U + local.Minute;
}

void printLocalTime(time_t value, long utcOffset) {
  uint16_t minutes = localMinutes(value, utcOffset);
  printTwoDigits(minutes / 60);
  Watchy::display.print(':');
  printTwoDigits(minutes % 60);
}

void drawSunRise(const SunRise &sun, long utcOffset) {
  WatchyUi::Screen::begin("SUN RISE");
  WatchyUi::Widget::separator();
  if (!sun.hasRise && !sun.hasSet) {
    AppVisual::drawEmptyState({8, 42, 184, 116}, "NO SUN EVENTS",
                              "No rise or set today");
    WatchyUi::Widget::footer("UP/DOWN DETAILS  BACK EXIT");
    finishAppDisplay();
    return;
  }
  uint16_t foreground = WatchyUi::Theme::foreground();
  Watchy::display.drawLine(12, 114, 188, 114, foreground);
  if (sun.isVisible) {
    Watchy::display.fillCircle(100, 91, 17, foreground);
    Watchy::display.fillCircle(100, 91, 11, WatchyUi::Theme::background());
  } else {
    Watchy::display.drawCircle(100, 105, 14, foreground);
  }

  Watchy::display.setFont();
  Watchy::display.setTextSize(1);
  Watchy::display.setTextColor(foreground);
  Watchy::display.setCursor(12, 45);
  Watchy::display.print(sun.hasRise && sun.hasSet ? "DAY WINDOW" : "NO EVENTS");
  Watchy::display.setCursor(12, 133);
  Watchy::display.print("RISE ");
  if (sun.hasRise) printLocalTime(sun.riseTime, utcOffset);
  else Watchy::display.print("--:--");
  Watchy::display.setCursor(112, 133);
  Watchy::display.print("SET ");
  if (sun.hasSet) printLocalTime(sun.setTime, utcOffset);
  else Watchy::display.print("--:--");

  WatchyUi::GrayPaint::fillRoundRect(
      WatchyUi::Bounds{12, 151, 176, 12}, 3,
      WatchyUi::Theme::tone(WatchyUi::ToneRole::SurfaceRaised));
  if (sun.hasRise && sun.hasSet) {
    int16_t start = 12 + localMinutes(sun.riseTime, utcOffset) * 176 / (24 * 60);
    int16_t end = 12 + localMinutes(sun.setTime, utcOffset) * 176 / (24 * 60);
    if (end > start) {
      Watchy::display.fillRect(start, 154, end - start, 6, foreground);
    }
  }
  Watchy::display.setCursor(12, 174);
  Watchy::display.print("AZ ");
  if (sun.hasRise) Watchy::display.print(sun.riseAz, 0);
  else Watchy::display.print("--");
  Watchy::display.print(" / ");
  if (sun.hasSet) Watchy::display.print(sun.setAz, 0);
  else Watchy::display.print("--");
  Watchy::display.print(" deg");
  WatchyUi::Widget::footer("UP/DOWN DETAILS  BACK EXIT");
  finishAppDisplay();
}

void drawSunRiseDetails(const SunRise &sun, long utcOffset) {
  beginCompactAppDisplay("SUN DETAILS");
  setCompactLine(0);
  Watchy::display.print("Query  ");
  printIsoDateTime(sun.queryTime, utcOffset);
  setCompactLine(1);
  Watchy::display.print("Rise az  ");
  if (sun.hasRise) {
    Watchy::display.print(sun.riseAz, 6);
    Watchy::display.print(" deg N");
  } else {
    Watchy::display.print("n/a");
  }
  setCompactLine(2);
  Watchy::display.print("Set az   ");
  if (sun.hasSet) {
    Watchy::display.print(sun.setAz, 6);
    Watchy::display.print(" deg N");
  } else {
    Watchy::display.print("n/a");
  }
  setCompactLine(4);
  Watchy::display.print("Visible  ");
  Watchy::display.print(sun.isVisible ? "yes" : "no");
  setCompactLine(5);
  Watchy::display.print("Has rise ");
  Watchy::display.print(sun.hasRise ? "yes" : "no");
  setCompactLine(6);
  Watchy::display.print("Has set  ");
  Watchy::display.print(sun.hasSet ? "yes" : "no");
  WatchyUi::Widget::footer("UP/DOWN SUMMARY  BACK EXIT");
  finishAppDisplay();
}

} // namespace

void showSunRiseImpl(Watchy *watchy) {
  tmElements_t currentTime;
  long utcOffset = gmtOffset;
  watchySettings watchSettings;
  if (watchy != nullptr) {
    watchy->RTC.read(watchy->currentTime);
    currentTime = watchy->currentTime;
    utcOffset = watchy->settings.gmtOffset;
    watchSettings = watchy->settings;
  } else {
    WatchySdk::RTC.read(WatchySdk::currentTime);
    currentTime = WatchySdk::currentTime;
    watchSettings = WatchySdk::settings;
  }
  time_t utcTime = makeTime(currentTime) - utcOffset;
  SunRise sun;
  sun.calculate(watchSettings.lat.toDouble(), watchSettings.lon.toDouble(),
                utcTime);
  WatchyUi::Input::begin();
  bool details = false;
  while (true) {
    if (details) drawSunRiseDetails(sun, utcOffset);
    else drawSunRise(sun, utcOffset);
    WatchyUi::Event event = WatchyUi::Input::wait();
    if (event == WatchyUi::Event::BACK) {
      if (watchy != nullptr) {
        watchy->showMenu(menuIndex, false);
      } else {
        WatchySdk::showMenu(menuIndex, false);
      }
      return;
    }
    if (event == WatchyUi::Event::UP || event == WatchyUi::Event::DOWN) {
      details = !details;
    }
  }
}

void Watchy::showSunRise() { showSunRiseImpl(this); }

void WatchySdk::showSunRise() { showSunRiseImpl(nullptr); }

#ifdef WATCHY_DETERMINISTIC_GALLERY
namespace WatchyDemo {

void renderSunRisePreview(uint8_t view, const tmElements_t &fixedTime) {
  SunRise sun;
  sun.calculate(40.4168, -3.7038, makeTime(fixedTime) - 2 * SECS_PER_HOUR);
  if (view != 0) {
    sun.hasRise = false;
    sun.hasSet = false;
    sun.isVisible = false;
  }
  long previousOffset = gmtOffset;
  gmtOffset = 2 * SECS_PER_HOUR;
  drawSunRise(sun, gmtOffset);
  gmtOffset = previousOffset;
}

} // namespace WatchyDemo
#endif
