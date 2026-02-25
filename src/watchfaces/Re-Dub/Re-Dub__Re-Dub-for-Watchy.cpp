#if defined(WATCHY_WRAPPER_INCLUDE) || defined(WATCHY_STANDALONE_WATCHFACE)
#include "../../watchy/Watchy.h" //include the Watchy library
#include "../../sdk/UiSDK.h"
#include "background.h"
#include "Technology40pt7b.h"
#include "Technology18pt7b.h"
        

class ReDubWatchFace : public Watchy { //inherit and extend Watchy class
  public:
    void drawWatchFace() { //override this method to customize how the watch face looks

      UiSDK::initScreen(display);
      const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);
      const uint16_t bgColor = UiSDK::getWatchfaceBg(BASE_POLARITY);
      
      int16_t  x1, y1;
      uint16_t w, h;
      String textstring, texthold;
      
      //drawbg
      UiSDK::drawBitmap(display, 0, 0, background, 200, 200, fgColor);
      
      UiSDK::setTextColor(display, bgColor);
      UiSDK::setTextWrap(display, false);

      //draw date
      UiSDK::setFont(display, &Technology18pt7b);
      textstring = dayShortStr(currentTime.Wday);
      textstring.toUpperCase();
      textstring += " ";
      textstring += currentTime.Day;
      UiSDK::getTextBounds(display, textstring, 0, 0, &x1, &y1, &w, &h);
      UiSDK::setCursor(display, 175-w, 90);
      UiSDK::print(display, textstring);

      //draw time
      UiSDK::setFont(display, &Technology40pt7b);

      texthold = currentTime.Hour;
      if (currentTime.Hour < 10) {
        textstring = "0";
      } else {
        textstring = texthold.charAt(0);
      }
      UiSDK::getTextBounds(display, textstring, 0, 0, &x1, &y1, &w, &h);
      UiSDK::setCursor(display, 58-w, 152);
      UiSDK::print(display, textstring);

      if (currentTime.Hour < 10) {
        textstring = texthold.charAt(0);
      } else {
        textstring = texthold.charAt(1);
      }
      UiSDK::getTextBounds(display, textstring, 0, 0, &x1, &y1, &w, &h);
      UiSDK::setCursor(display, 91-w, 152);
      UiSDK::print(display, textstring);

      textstring = ":";
      UiSDK::getTextBounds(display, textstring, 0, 0, &x1, &y1, &w, &h);
      UiSDK::setCursor(display, 100-w/2, 152);
      UiSDK::print(display, textstring);

      texthold = currentTime.Minute; 
      if (currentTime.Minute < 10) {
        textstring = "0";
      } else {
        textstring = texthold.charAt(0);
      }
      UiSDK::getTextBounds(display, textstring, 0, 0, &x1, &y1, &w, &h);
      UiSDK::setCursor(display, 142-w, 152);
      UiSDK::print(display, textstring);

      if (currentTime.Minute < 10) {
        textstring = texthold.charAt(0);
      } else {
        textstring = texthold.charAt(1);
      }
      UiSDK::getTextBounds(display, textstring, 0, 0, &x1, &y1, &w, &h);
      UiSDK::setCursor(display, 175-w, 152);
      UiSDK::print(display, textstring);


      // draw battery
      float batt = (getBatteryVoltage()-3.3)/0.9;
      if (batt > 0) {
       UiSDK::fillRect(display, 160,53,15*batt,6,bgColor);
      }                 
    }
};

void WatchfaceEntrypoint_ReDub(Watchy &watchy) {
  ReDubWatchFace face;
  face.settings = watchy.settings;
  face.currentTime = watchy.currentTime;
  face.drawWatchFace();
}

void showWatchFace_Re_Dub(Watchy &watchy) {
  WatchfaceEntrypoint_ReDub(watchy);
}

#ifdef WATCHY_STANDALONE_WATCHFACE
ReDubWatchFace m; //instantiate your watchface

void setup() {
  m.init(); //call init in setup
}

void loop() {
  // this should never run, Watchy deep sleeps after init();
}
#endif

#endif // WATCHY_WRAPPER_INCLUDE || WATCHY_STANDALONE_WATCHFACE