#if defined(WATCHY_WRAPPER_INCLUDE) || defined(WATCHY_STANDALONE_WATCHFACE)
/*
SmartWatchy
https://github.com/theRealc2c2/SmartWatch

Based on Bahn-for-Watchy
https://github.com/BraininaBowl/Bahn-for-Watchy

Face for Watchy watch
https://watchy.sqfmi.com
*/

#include "../../watchy/Watchy.h" //include the Watchy library
#include "../../sdk/UiSDK.h"
#include "Teko_Regular12pt7b.h"
#include "Teko_Regular50pt7b.h"
#include "icons.h"

class SmartWatchyWatchFace : public Watchy { //inherit and extend Watchy class
  using Watchy::Watchy;
  public:
    void drawWatchFace() { //override this method to customize how the watch face looks
      
      int16_t  x1, y1;
      uint16_t w, h;
      String textstring;

      UiSDK::initScreen(display);
      const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);
      const uint16_t bgColor = UiSDK::getWatchfaceBg(BASE_POLARITY);

      // Step counter managed by core Watchy

      //drawbg
      UiSDK::setFont(display, &Teko_Regular50pt7b);
  UiSDK::setTextColor(display, fgColor);
      UiSDK::setTextWrap(display, false);

      //draw image
  UiSDK::drawBitmap(display, 16,159, epd_bitmap_icons, 168, 41, fgColor);

      //draw time
      textstring = currentTime.Hour;
      textstring += ":";
      if (currentTime.Minute < 10) {
        textstring += "0";
      } else {
        textstring += "";
      }
      textstring += currentTime.Minute;
      UiSDK::getTextBounds(display, textstring, 0, 0, &x1, &y1, &w, &h);
      UiSDK::setCursor(display, 100-w/2, 110);
      UiSDK::print(display, textstring);

      // draw battery
      UiSDK::fillRoundRect(display, 16,16,34,12,4,fgColor);
      UiSDK::fillRoundRect(display, 49,20,3,4,2,fgColor);
      UiSDK::fillRoundRect(display, 18,18,30,8,3,bgColor);
      float batt = (getBatteryVoltage()-3.3)/0.9;
      if (batt > 0) {
       UiSDK::fillRoundRect(display, 20,20,26*batt,4,2,fgColor);
      }										

      UiSDK::setFont(display, &Teko_Regular12pt7b);

      //draw steps
      textstring = sensor.getCounter();
      textstring += " steps";
      UiSDK::getTextBounds(display, textstring, 0, 0, &x1, &y1, &w, &h);
      UiSDK::setCursor(display, 155-w/2, 145);
      UiSDK::setTextColor(display, fgColor);
      UiSDK::print(display, textstring);

      // draw date
      textstring = monthShortStr(currentTime.Month);
      textstring += " ";
      textstring += currentTime.Day;
      textstring += " ";
      textstring += currentTime.Year + 1970;
      UiSDK::getTextBounds(display, textstring, 0, 0, &x1, &y1, &w, &h);
      UiSDK::setCursor(display, 50-w/2, 145);
      UiSDK::print(display, textstring);

    }

};

void WatchfaceEntrypoint_SmartWatchy(Watchy &watchy) {
  SmartWatchyWatchFace face;
  face.settings = watchy.settings;
  face.currentTime = watchy.currentTime;
  face.drawWatchFace();
}

void showWatchFace_SmartWatchy(Watchy &watchy) {
  WatchfaceEntrypoint_SmartWatchy(watchy);
}

#endif // WATCHY_WRAPPER_INCLUDE || WATCHY_STANDALONE_WATCHFACE


