#include <MoonPhase.h>
#include "AppDisplay.h"

namespace {

void drawMoonPhase(const MoonPhase &moon) {
  beginAppDisplay("MOON PHASE");
  Watchy::display.setCursor(0, 35);
  Watchy::display.print("Julian: ");
  Watchy::display.println(moon.jDate, 2);
  Watchy::display.setCursor(0, 55);
  Watchy::display.print("Phase: ");
  Watchy::display.println(moon.phase, 3);
  Watchy::display.setCursor(0, 75);
  Watchy::display.print("Age: ");
  Watchy::display.print(moon.age, 2);
  Watchy::display.println(" d");
  Watchy::display.setCursor(0, 95);
  Watchy::display.print("Light: ");
  Watchy::display.print(moon.fraction * 100.0, 1);
  Watchy::display.println('%');
  Watchy::display.setCursor(0, 115);
  Watchy::display.print("Dist: ");
  Watchy::display.print(moon.distance, 2);
  Watchy::display.println(" ER");
  Watchy::display.setCursor(0, 135);
  Watchy::display.print("Lat: ");
  Watchy::display.println(moon.latitude, 2);
  Watchy::display.setCursor(0, 155);
  Watchy::display.print("Lon: ");
  Watchy::display.println(moon.longitude, 2);
  Watchy::display.setCursor(0, 175);
  Watchy::display.print("Name: ");
  Watchy::display.println(moon.phaseName);
  Watchy::display.setCursor(0, 195);
  Watchy::display.print("Zodiac: ");
  Watchy::display.println(moon.zodiacName);
  finishAppDisplay();
}

} // namespace

void Watchy::showMoonPhase() {
  RTC.read(currentTime);
  MoonPhase moon;
  moon.calculate(currentUtcTime(currentTime));
  drawMoonPhase(moon);
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
namespace WatchyDemo {

void renderMoonPhasePreview(const tmElements_t &fixedTime) {
  MoonPhase moon;
  moon.calculate(makeTime(fixedTime) - 2 * SECS_PER_HOUR);
  drawMoonPhase(moon);
}

} // namespace WatchyDemo
#endif