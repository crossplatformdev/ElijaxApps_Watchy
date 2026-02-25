#include <SunRise.h>
#include "AppDisplay.h"

namespace {

void drawSunRise(const SunRise &sun) {
  beginAppDisplay("SUN");
  Watchy::display.setCursor(0, 36);
  Watchy::display.print("Query: ");
  printDateTime(sun.queryTime, gmtOffset);
  Watchy::display.setCursor(0, 57);
  Watchy::display.print("Rise:  ");
  if (sun.hasRise) {
    printDateTime(sun.riseTime, gmtOffset);
  } else {
    Watchy::display.print("--");
  }
  Watchy::display.setCursor(0, 78);
  Watchy::display.print("Rise az: ");
  Watchy::display.print(sun.riseAz, 1);
  Watchy::display.println(" deg");
  Watchy::display.setCursor(0, 99);
  Watchy::display.print("Set:   ");
  if (sun.hasSet) {
    printDateTime(sun.setTime, gmtOffset);
  } else {
    Watchy::display.print("--");
  }
  Watchy::display.setCursor(0, 120);
  Watchy::display.print("Set az:  ");
  Watchy::display.print(sun.setAz, 1);
  Watchy::display.println(" deg");
  Watchy::display.setCursor(0, 141);
  Watchy::display.print("Visible: ");
  Watchy::display.println(sun.isVisible ? "yes" : "no");
  Watchy::display.setCursor(0, 162);
  Watchy::display.print("Has rise: ");
  Watchy::display.println(sun.hasRise ? "yes" : "no");
  Watchy::display.setCursor(0, 183);
  Watchy::display.print("Has set:  ");
  Watchy::display.println(sun.hasSet ? "yes" : "no");
  finishAppDisplay();
}

} // namespace

void Watchy::showSunRise() {
  RTC.read(currentTime);
  time_t utcTime = currentUtcTime(currentTime);
  SunRise sun;
  sun.calculate(settings.lat.toDouble(), settings.lon.toDouble(), utcTime);
  drawSunRise(sun);
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
namespace WatchyDemo {

void renderSunRisePreview(const tmElements_t &fixedTime) {
  SunRise sun;
  sun.calculate(40.4168, -3.7038, makeTime(fixedTime) - 2 * SECS_PER_HOUR);
  long previousOffset = gmtOffset;
  gmtOffset = 2 * SECS_PER_HOUR;
  drawSunRise(sun);
  gmtOffset = previousOffset;
}

} // namespace WatchyDemo
#endif