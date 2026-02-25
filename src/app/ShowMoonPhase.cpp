#include <MoonPhase.h>
#include "AppDisplay.h"

#include "WatchyUi.h"
#include "Watchy.h"

namespace {

void drawMoonPhase(const MoonPhase &moon) {
  WatchyUi::Screen::begin("MOON PHASE");
  WatchyUi::Widget::separator();
  uint16_t foreground = WatchyUi::Theme::foreground();
  uint16_t background = WatchyUi::Theme::background();
  constexpr int16_t moonX = 53;
  constexpr int16_t moonY = 91;
  constexpr int16_t moonRadius = 34;
  Watchy::display.drawCircle(moonX, moonY, moonRadius, foreground);
  Watchy::display.fillCircle(moonX, moonY, moonRadius - 1, foreground);
  float phase = constrain(static_cast<float>(moon.phase), 0.0f, 1.0f);
  int16_t shadowOffset = static_cast<int16_t>((phase < 0.5f
      ? 1.0f - phase * 2.0f : (phase - 0.5f) * 2.0f) * moonRadius * 2);
  Watchy::display.fillCircle(phase < 0.5f ? moonX - shadowOffset
                                          : moonX + shadowOffset,
                             moonY, moonRadius - 1, background);
  Watchy::display.setFont();
  Watchy::display.setTextSize(1);
  Watchy::display.setTextColor(foreground);
  WatchyUi::Canvas::centeredText(
      WatchyUi::Bounds{8, 132, 90, 14}, moon.phaseName, foreground);
  WatchyUi::Canvas::centeredText(
      WatchyUi::Bounds{8, 147, 90, 14}, moon.zodiacName, foreground);

  constexpr WatchyUi::Bounds litBounds{108, 42, 80, 65};
  WatchyUi::GrayPaint::fillRoundRect(
      litBounds, 3, WatchyUi::Theme::tone(WatchyUi::ToneRole::Surface));
  WatchyUi::Canvas::centeredText(
      {litBounds.x, litBounds.y, litBounds.width, 14}, "LIT", 1, foreground);
  char percentText[6];
  snprintf(percentText, sizeof(percentText), "%.0f%%", moon.fraction * 100.0);
  WatchyUi::Canvas::centeredText(
      {litBounds.x, static_cast<int16_t>(litBounds.y + 14), litBounds.width, 28},
      percentText, 3, foreground);
  char ageText[16];
  snprintf(ageText, sizeof(ageText), "AGE %.1f d", moon.age);
  WatchyUi::Canvas::centeredText(
      {litBounds.x, static_cast<int16_t>(litBounds.y + 42), litBounds.width, 12},
      ageText, 1, foreground);
  char distText[18];
  snprintf(distText, sizeof(distText), "DIST %.2f ER", moon.distance);
  WatchyUi::Canvas::centeredText(
      {litBounds.x, static_cast<int16_t>(litBounds.y + 54), litBounds.width, 11},
      distText, 1, foreground);
  WatchyUi::Widget::progress(moon.fraction, 165);
  WatchyUi::Widget::footer("UP/DOWN DETAILS  BACK EXIT");
  finishAppDisplay();
}

void drawMoonPhaseDetails(const MoonPhase &moon) {
  beginCompactAppDisplay("MOON DETAILS");
  setCompactLine(0);
  Watchy::display.print("Julian       ");
  Watchy::display.print(moon.jDate, 6);
  setCompactLine(1);
  Watchy::display.print("Phase        ");
  Watchy::display.print(moon.phase, 6);
  setCompactLine(2);
  Watchy::display.print("Age          ");
  Watchy::display.print(moon.age, 6);
  Watchy::display.print(" d");
  setCompactLine(3);
  Watchy::display.print("Illuminated  ");
  Watchy::display.print(moon.fraction * 100.0, 6);
  Watchy::display.print(" %");
  setCompactLine(4);
  Watchy::display.print("Distance     ");
  Watchy::display.print(moon.distance, 6);
  Watchy::display.print(" ER");
  setCompactLine(5);
  Watchy::display.print("Ecliptic lat ");
  Watchy::display.print(moon.latitude, 6);
  Watchy::display.print(" deg");
  setCompactLine(6);
  Watchy::display.print("Ecliptic lon ");
  Watchy::display.print(moon.longitude, 6);
  Watchy::display.print(" deg");
  setCompactLine(7);
  Watchy::display.print("Phase name   ");
  Watchy::display.print(moon.phaseName);
  setCompactLine(8);
  Watchy::display.print("Zodiac       ");
  Watchy::display.print(moon.zodiacName);
  WatchyUi::Widget::footer("UP/DOWN SUMMARY  BACK EXIT");
  finishAppDisplay();
}

} // namespace

void showMoonPhaseImpl(Watchy *watchy) {
  tmElements_t currentTime;
  if (watchy != nullptr) {
    watchy->RTC.read(watchy->currentTime);
    currentTime = watchy->currentTime;
  } else {
    WatchySdk::RTC.read(WatchySdk::currentTime);
    currentTime = WatchySdk::currentTime;
  }
  MoonPhase moon;
  moon.calculate(currentUtcTime(currentTime));
  WatchyUi::Input::begin();
  bool details = false;
  while (true) {
    if (details) drawMoonPhaseDetails(moon);
    else drawMoonPhase(moon);
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

void Watchy::showMoonPhase() { showMoonPhaseImpl(this); }

void WatchySdk::showMoonPhase() { showMoonPhaseImpl(nullptr); }

#ifdef WATCHY_DETERMINISTIC_GALLERY
namespace WatchyDemo {

void renderMoonPhasePreview(uint8_t view, const tmElements_t &fixedTime) {
  (void)view;
  MoonPhase moon;
  moon.calculate(makeTime(fixedTime) - 2 * SECS_PER_HOUR);
  drawMoonPhase(moon);
}

} // namespace WatchyDemo
#endif
