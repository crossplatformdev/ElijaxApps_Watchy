#include "../../watchy/Watchy.h"
#include "DSEG7_Classic_Regular_39.h"
#include "../../sdk/UiSDK.h"

namespace {

void drawHand(Watchy &watchy, uint16_t fgColor, uint8_t _radius_, float _angle_) {
  float x = _radius_*cos(_angle_);
  float y = _radius_*sin(_angle_);
  UiSDK::drawLine(watchy.display, 99, 99, 99+x, 99+y, fgColor);
  UiSDK::drawLine(watchy.display, 99, 100, 99+x, 100+y, fgColor);
  UiSDK::drawLine(watchy.display, 99, 101, 99+x, 101+y, fgColor);
  UiSDK::drawLine(watchy.display, 100, 99, 100+x, 99+y, fgColor);
  UiSDK::drawLine(watchy.display, 100, 100, 100+x, 100+y, fgColor);
  UiSDK::drawLine(watchy.display, 100, 101, 100+x, 101+y, fgColor);
  UiSDK::drawLine(watchy.display, 101, 99, 101+x, 99+y, fgColor);
  UiSDK::drawLine(watchy.display, 101, 100, 101+x, 100+y, fgColor);
  UiSDK::drawLine(watchy.display, 101, 101, 101+x, 101+y, fgColor);
}

} // namespace

void showWatchFace_Line(Watchy &watchy) {
  UiSDK::initScreen(watchy.display);
  const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);
  const uint16_t bgColor = UiSDK::getWatchfaceBg(BASE_POLARITY);
  
  uint8_t myHour;
  uint8_t myMinute;
  uint8_t radius;
  float angle_hourScale;
  float angle_minuteScale;
  float angle_hourHand;
  float angle_minuteHand;
  float pi = 3.1415926535897932384626433832795;
  
  UiSDK::setTextColor(watchy.display, fgColor);
  UiSDK::setFont(watchy.display, &DSEG7_Classic_Regular_39);
  
  myHour   = watchy.currentTime.Hour > 12 ? watchy.currentTime.Hour - 12 : watchy.currentTime.Hour;
  myMinute = watchy.currentTime.Minute;
  
  angle_hourScale   = 2*pi/12;
  angle_minuteScale = 2*pi/60;
  angle_hourHand    = angle_hourScale*(myHour-3)+2*pi/720*myMinute;
  angle_minuteHand  = angle_minuteScale*(myMinute-15);
  
  // draw minute scale
  radius = 98;
  for (uint8_t i=0; i<60; i++) {
    UiSDK::drawLine(watchy.display, 100, 100, 100+radius*cos(angle_minuteScale*i), 100+radius*sin(angle_minuteScale*i), fgColor);
  }
  UiSDK::fillCircle(watchy.display, 100, 100, 93, bgColor);

  // draw hour scale
  radius = 98;
  for (uint8_t i=0; i<12; i++) {
    drawHand(watchy, fgColor, radius, angle_hourScale*i);
  }
  UiSDK::fillCircle(watchy.display, 100, 100, 88, bgColor);

  // draw minute hand
  radius = 98;
  drawHand(watchy, fgColor, radius, angle_minuteHand);

  // draw center point
  UiSDK::fillCircle(watchy.display, 100, 100, 45, bgColor);

  // positioning of hour display for DSEG7_Classic_Regular_39 font
  if (watchy.currentTime.Hour > 9 && watchy.currentTime.Hour < 20) {
    UiSDK::setCursor(watchy.display, 58, 120);
  } else {
    UiSDK::setCursor(watchy.display, 68, 120);
  }
  
  // display hour (with a leading zero, if necessary)
  if(watchy.currentTime.Hour < 10){
      UiSDK::print(watchy.display, "0");
  }
  UiSDK::print(watchy.display, watchy.currentTime.Hour);
}
