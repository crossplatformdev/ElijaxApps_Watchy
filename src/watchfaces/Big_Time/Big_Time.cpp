#include "../../watchy/Watchy.h"
#include "../../sdk/UiSDK.h"
#include "Vollkorn_Black48pt7b.h"
#include "Vollkorn_Medium12pt7b.h"

void showWatchFace_BigTime(Watchy &watchy) {
  int16_t x1 = 0;
  int16_t y1 = 0;
  uint16_t w = 0;
  uint16_t h = 0;
  String textstring;

  UiSDK::initScreen(watchy.display);

  textstring = watchy.currentTime.Hour;
  textstring += ":";
  if (watchy.currentTime.Minute < 10) {
    textstring += "0";
  }
  textstring += watchy.currentTime.Minute;
  UiSDK::setFont(watchy.display, &Vollkorn_Black48pt7b);
  UiSDK::getTextBounds(watchy.display, textstring, 0, 0, &x1, &y1, &w, &h);

  UITextSpec timeText;
  timeText.x = static_cast<int16_t>(100 - (w / 2));
  timeText.y = 104;
  timeText.w = 0;
  timeText.h = 0;
  timeText.font = &Vollkorn_Black48pt7b;
  timeText.fillBackground = false;
  timeText.invert = false;
  timeText.text = textstring;
  UiSDK::renderText(watchy.display, timeText);

  textstring = monthStr(watchy.currentTime.Month);
  textstring += " ";
  textstring += watchy.currentTime.Day;

  if (watchy.currentTime.Day % 10 == 1) {
    textstring += "st ";
  } else if (watchy.currentTime.Day % 10 == 2) {
    textstring += "nd ";
  } else if (watchy.currentTime.Day % 10 == 3) {
    textstring += "rd ";
  } else {
    textstring += "th ";
  }
  textstring += watchy.currentTime.Year + 1970;
  UiSDK::setFont(watchy.display, &Vollkorn_Medium12pt7b);
  UiSDK::getTextBounds(watchy.display, textstring, 0, 0, &x1, &y1, &w, &h);

  UITextSpec dateText;
  dateText.x = static_cast<int16_t>(100 - (w / 2));
  dateText.y = static_cast<int16_t>(128 + h);
  dateText.w = 0;
  dateText.h = 0;
  dateText.font = &Vollkorn_Medium12pt7b;
  dateText.fillBackground = false;
  dateText.invert = false;
  dateText.text = textstring;
  UiSDK::renderText(watchy.display, dateText);

  float batt = (watchy.getBatteryVoltage() - 3.3f) / 0.9f;
  if (batt > 0) {
    const uint16_t fg = UiSDK::getWatchfaceFg(UiSDK::getThemePolarity());
    int16_t bw = static_cast<int16_t>(196 * batt);
    if (bw < 0) {
      bw = 0;
    }
    if (bw > 196) {
      bw = 196;
    }
    UiSDK::fillRect(watchy.display, 2, 196, bw, 2, fg);
  }
}
