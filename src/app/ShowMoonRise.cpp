#include <MoonRise.h>
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

void drawMoonRise(const MoonRise &moon, long utcOffset) {
  WatchyUi::Screen::begin("MOON RISE");
  WatchyUi::Widget::separator();
  if (!moon.hasRise && !moon.hasSet) {
    AppVisual::drawEmptyState({8, 42, 184, 116}, "NO MOON EVENTS",
                              "No rise or set today");
    WatchyUi::Widget::footer("UP/DOWN DETAILS  BACK EXIT");
    finishAppDisplay();
    return;
  }
  uint16_t foreground = WatchyUi::Theme::foreground();
  Watchy::display.drawLine(12, 114, 188, 114, foreground);
  Watchy::display.drawCircle(100, 91, 17, foreground);
  Watchy::display.fillCircle(107, 85, 17, WatchyUi::Theme::background());
  Watchy::display.setFont();
  Watchy::display.setTextSize(1);
  Watchy::display.setTextColor(foreground);
  Watchy::display.setCursor(12, 45);
  Watchy::display.print(moon.isVisible ? "ABOVE HORIZON" : "BELOW HORIZON");
  Watchy::display.setCursor(12, 133);
  Watchy::display.print("RISE ");
  if (moon.hasRise) printLocalTime(moon.riseTime, utcOffset);
  else Watchy::display.print("--:--");
  Watchy::display.setCursor(112, 133);
  Watchy::display.print("SET ");
  if (moon.hasSet) printLocalTime(moon.setTime, utcOffset);
  else Watchy::display.print("--:--");
  WatchyUi::GrayPaint::fillRoundRect(
      WatchyUi::Bounds{12, 151, 176, 12}, 3,
      WatchyUi::Theme::tone(WatchyUi::ToneRole::SurfaceRaised));
  if (moon.hasRise && moon.hasSet) {
    int16_t start = 12 + localMinutes(moon.riseTime, utcOffset) * 176 / (24 * 60);
    int16_t end = 12 + localMinutes(moon.setTime, utcOffset) * 176 / (24 * 60);
    if (end > start) Watchy::display.fillRect(start, 154, end - start, 6,
                                              foreground);
  }
  Watchy::display.setCursor(12, 174);
  Watchy::display.print("AZ ");
  if (moon.hasRise) Watchy::display.print(moon.riseAz, 0);
  else Watchy::display.print("--");
  Watchy::display.print(" / ");
  if (moon.hasSet) Watchy::display.print(moon.setAz, 0);
  else Watchy::display.print("--");
  Watchy::display.print(" deg");
  WatchyUi::Widget::footer("UP/DOWN DETAILS  BACK EXIT");
  finishAppDisplay();
}

void drawMoonRiseDetails(const MoonRise &moon, long utcOffset) {
  beginCompactAppDisplay("MOON DETAILS");
  setCompactLine(0);
  Watchy::display.print("Query  ");
  printIsoDateTime(moon.queryTime, utcOffset);
  setCompactLine(1);
  Watchy::display.print("Rise az  ");
  if (moon.hasRise) {
    Watchy::display.print(moon.riseAz, 6);
    Watchy::display.print(" deg N");
  } else {
    Watchy::display.print("n/a");
  }
  setCompactLine(2);
  Watchy::display.print("Set az   ");
  if (moon.hasSet) {
    Watchy::display.print(moon.setAz, 6);
    Watchy::display.print(" deg N");
  } else {
    Watchy::display.print("n/a");
  }
  setCompactLine(4);
  Watchy::display.print("Visible  ");
  Watchy::display.print(moon.isVisible ? "yes" : "no");
  setCompactLine(5);
  Watchy::display.print("Has rise ");
  Watchy::display.print(moon.hasRise ? "yes" : "no");
  setCompactLine(6);
  Watchy::display.print("Has set  ");
  Watchy::display.print(moon.hasSet ? "yes" : "no");
  WatchyUi::Widget::footer("UP/DOWN SUMMARY  BACK EXIT");
  finishAppDisplay();
}

} // namespace

void showMoonRiseImpl(Watchy *watchy) {
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
  MoonRise moon;
  moon.calculate(watchSettings.lat.toDouble(), watchSettings.lon.toDouble(),
                 utcTime);
  WatchyUi::Input::begin();
  bool details = false;
  while (true) {
    if (details) drawMoonRiseDetails(moon, utcOffset);
    else drawMoonRise(moon, utcOffset);
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

void Watchy::showMoonRise() { showMoonRiseImpl(this); }

void WatchySdk::showMoonRise() { showMoonRiseImpl(nullptr); }

#ifdef WATCHY_DETERMINISTIC_GALLERY
namespace WatchyDemo {

void renderMoonRisePreview(uint8_t view, const tmElements_t &fixedTime) {
  MoonRise moon;
  moon.calculate(40.4168, -3.7038,
                 makeTime(fixedTime) - 2 * SECS_PER_HOUR);
  if (view != 0) {
    moon.hasRise = false;
    moon.hasSet = false;
    moon.isVisible = false;
  }
  long previousOffset = gmtOffset;
  gmtOffset = 2 * SECS_PER_HOUR;
  drawMoonRise(moon, gmtOffset);
  gmtOffset = previousOffset;
}

} // namespace WatchyDemo
#endif
