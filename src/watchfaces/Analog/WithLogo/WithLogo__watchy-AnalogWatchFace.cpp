#include "../../../watchy/Watchy.h" //include the Watchy library
#include "clock_face.h"
#include <Fonts/FreeSerifBold12pt7b.h>
#include <Fonts/FreeSerifItalic9pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include "sdk/UiSDK.h"

class MyFirstWatchFace : public Watchy{ //inherit and extend Watchy class
    public:
        void drawWatchFace(){ //override this method to customize how the watch face looks
          
          //background
          UiSDK::drawBitmap(display, 0, 0, clock_face, 200, 200, GxEPD_WHITE);
          UiSDK::fillCircle(display, 100,100,3,GxEPD_WHITE);

          //Logo
          UiSDK::setCursor(display, 77, 45);
          UiSDK::setFont(display, &FreeSansBold9pt7b);
          UiSDK::print(display, "Gabel ");
          UiSDK::setCursor(display, 55, 65);
          UiSDK::setFont(display, &FreeSerifItalic9pt7b);
          UiSDK::print(display, "Chronometrie");

          //date
          String dateDay = "";
          if(currentTime.Day < 10){
            dateDay += "0";
          }
          dateDay += currentTime.Day;
          UiSDK::fillRect(display, 128, 88, 26, 24, GxEPD_WHITE);
          UiSDK::setFont(display, &FreeSerifBold12pt7b);
          UiSDK::setTextColor(display, GxEPD_BLACK);
          UiSDK::setCursor(display, 129, 107);
          UiSDK::print(display, dateDay);

          // draw battery
          UiSDK::drawRect(display, 90, 140, 19, 11, GxEPD_WHITE);
          UiSDK::fillRect(display, 110, 143, 2, 5, GxEPD_WHITE);
          float battery = (getBatteryVoltage()-3.3)/0.9;
          if (battery > 0) {
            UiSDK::fillRect(display, 91, 141, 19*battery, 10, GxEPD_WHITE);

          //minute pointer
          int currentMinute = currentTime.Minute;
          int minuteAngle = currentMinute * 6;
          double radMinute = ((minuteAngle + 180) * 71) / 4068.0;
          double mx1 = 100 - (sin(radMinute) * 85);
          double my1 = 100 + (cos(radMinute) * 85);
          UiSDK::drawLine(display, 100, 100, (int)mx1, (int)my1, GxEPD_WHITE);

          //hour pointer
          int currentHour= currentTime.Hour;
          double hourAngle = (currentHour * 30) + currentMinute * 0.5;
          double radHour = ((hourAngle + 180) * 71) / 4068.0;
          double hx1 = 100 - (sin(radHour) * 45);
          double hy1 = 100 + (cos(radHour) * 45);
          UiSDK::drawLine(display, 100, 100, (int)hx1, (int)hy1, GxEPD_WHITE);

          }

        }
};

static MyFirstWatchFace m; //instantiate your watchface



#ifdef WATCHY_STANDALONE_WATCHFACE
void setup() {
  m.init(); //call init in setup
}

void loop() {
  // this should never run, Watchy deep sleeps after init();
}

#endif
