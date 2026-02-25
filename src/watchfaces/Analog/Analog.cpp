#include "../../watchy/Watchy.h"
#include "../../sdk/UiSDK.h"
#include "clock_face.h"
#include <Fonts/FreeSerifBold12pt7b.h>
#include <Fonts/FreeSerifItalic9pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>

void showWatchFace_Analog(Watchy &watchy) {
  UiSDK::initScreen(watchy.display);
  const uint16_t bg = UiSDK::getWatchfaceBg(BASE_POLARITY);
  const uint16_t fg = UiSDK::getWatchfaceFg(BASE_POLARITY);

  // background
  UIImageSpec face{};
  face.bitmap = clock_face;
  face.x = 0;
  face.y = 0;
  face.w = 200;
  face.h = 200;
  face.fromProgmem = true;
  face.fillBackground = false;
  UiSDK::renderImage(watchy.display, face);
  UiSDK::fillCircle(watchy.display, 100, 100, 3, fg);

  // date
  String dateDay;
  if (watchy.currentTime.Day < 10) {
    dateDay = "0";
  }
  dateDay += watchy.currentTime.Day;
  UiSDK::fillRect(watchy.display, 128, 88, 26, 24, fg);
  UiSDK::setFont(watchy.display, &FreeSerifBold12pt7b);
  UiSDK::setTextColor(watchy.display, bg);
  UiSDK::setCursor(watchy.display, 129, 107);
  UiSDK::print(watchy.display, dateDay);

  // battery
  UiSDK::drawRect(watchy.display, 90, 140, 19, 11, fg);
  UiSDK::fillRect(watchy.display, 110, 143, 2, 5, fg);
  float battery = (watchy.getBatteryVoltage() - 3.3f) / 0.9f;
  if (battery < 0) {
    battery = 0;
  }
  if (battery > 1) {
    battery = 1;
  }
  if (battery > 0) {
    UiSDK::fillRect(watchy.display, 91, 141, static_cast<int16_t>(19 * battery), 10, fg);
  }

  // minute pointer
  const int currentMinute = watchy.currentTime.Minute;
  const int minuteAngle = currentMinute * 6;
  const double radMinute = ((minuteAngle + 180) * 71) / 4068.0;
  const double mx1 = 100 - (sin(radMinute) * 85);
  const double my1 = 100 + (cos(radMinute) * 85);
  UiSDK::drawLine(watchy.display, 100, 100, static_cast<int>(mx1), static_cast<int>(my1), fg);

  // hour pointer
  const int currentHour = watchy.currentTime.Hour;
  const double hourAngle = (currentHour * 30) + currentMinute * 0.5;
  const double radHour = ((hourAngle + 180) * 71) / 4068.0;
  const double hx1 = 100 - (sin(radHour) * 45);
  const double hy1 = 100 + (cos(radHour) * 45);
  UiSDK::drawLine(watchy.display, 100, 100, static_cast<int>(hx1), static_cast<int>(hy1), fg);
}
