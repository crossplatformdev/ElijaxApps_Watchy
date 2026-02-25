#include <MoonRise.h>
#include "AppDisplay.h"

namespace {

void drawMoonRise(const MoonRise &moon) {
  beginAppDisplay("MOON RISE");
  Watchy::display.setCursor(0, 36);
  Watchy::display.print("Query: ");
  printDateTime(moon.queryTime, gmtOffset);
  Watchy::display.setCursor(0, 57);
  Watchy::display.print("Rise:  ");
  if (moon.hasRise) {
    printDateTime(moon.riseTime, gmtOffset);
  } else {
    Watchy::display.print("--");
  }
  Watchy::display.setCursor(0, 78);
  Watchy::display.print("Rise az: ");
  Watchy::display.print(moon.riseAz, 1);
  Watchy::display.println(" deg");
  Watchy::display.setCursor(0, 99);
  Watchy::display.print("Set:   ");
  if (moon.hasSet) {
    printDateTime(moon.setTime, gmtOffset);
  } else {
    Watchy::display.print("--");
  }
  Watchy::display.setCursor(0, 120);
  Watchy::display.print("Set az:  ");
  Watchy::display.print(moon.setAz, 1);
  Watchy::display.println(" deg");
  Watchy::display.setCursor(0, 141);
  Watchy::display.print("Visible: ");
  Watchy::display.println(moon.isVisible ? "yes" : "no");
  Watchy::display.setCursor(0, 162);
  Watchy::display.print("Has rise: ");
  Watchy::display.println(moon.hasRise ? "yes" : "no");
  Watchy::display.setCursor(0, 183);
  Watchy::display.print("Has set:  ");
  Watchy::display.println(moon.hasSet ? "yes" : "no");
  finishAppDisplay();
}

} // namespace

void Watchy::showMoonRise() {
  RTC.read(currentTime);
  time_t utcTime = currentUtcTime(currentTime);
  MoonRise moon;
  moon.calculate(settings.lat.toDouble(), settings.lon.toDouble(), utcTime);
  drawMoonRise(moon);
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
namespace WatchyDemo {

void renderMoonRisePreview(const tmElements_t &fixedTime) {
  MoonRise moon;
  moon.calculate(40.4168, -3.7038,
                 makeTime(fixedTime) - 2 * SECS_PER_HOUR);
  long previousOffset = gmtOffset;
  gmtOffset = 2 * SECS_PER_HOUR;
  drawMoonRise(moon);
  gmtOffset = previousOffset;
}

} // namespace WatchyDemo
#endif