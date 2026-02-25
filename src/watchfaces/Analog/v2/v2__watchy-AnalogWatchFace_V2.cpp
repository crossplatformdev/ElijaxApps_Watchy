#include "../../../watchy/Watchy.h" //include the Watchy library
#include "clockFace_square2.h"
#include <Fonts/FreeSerifBoldItalic9pt7b.h>
#include <Fonts/FreeSerifItalic9pt7b.h>
#include <Fonts/FreeSerifBold12pt7b.h>
#include "FreeSerifBoldItalic4pt7b.h"

#include "../../../watchy/Watchy.h" //include the Watchy library
#include <Fonts/FreeMonoOblique24pt7b.h> //include any fonts you want to use
#include "sdk/UiSDK.h"

class MyFirstWatchFace : public Watchy{ //inherit and extend Watchy class
    public:
        void drawWatchFace(){ //override this method to customize how the watch face looks
          
          //background
          UiSDK::drawBitmap(display, 0, 0, clockFace_square2, 200, 200, GxEPD_WHITE);
          UiSDK::fillCircle(display, 100,100,3,GxEPD_WHITE);
   
          //Logo
          UiSDK::setCursor(display, 80, 40);
          UiSDK::setFont(display, &FreeSerifBoldItalic9pt7b);
          UiSDK::print(display, "Gabel ");
          UiSDK::setCursor(display, 54, 60);
          UiSDK::setFont(display, &FreeSerifItalic9pt7b);
          UiSDK::print(display, "Chronometrie");

          //date
          String dateDay = "";
          if(currentTime.Day < 10){
            dateDay += "0";
          }
          dateDay += currentTime.Day;
          UiSDK::fillRect(display, 128, 88, 27, 24, GxEPD_WHITE);
          UiSDK::setFont(display, &FreeSerifBold12pt7b);
          UiSDK::setTextColor(display, GxEPD_BLACK);
          UiSDK::setCursor(display, 129, 107);
          UiSDK::print(display, dateDay);

          //weekday
          String wDay = dayShortStr(currentTime.Wday);
          wDay = wDay.substring(0,wDay.length() - 1);
          UiSDK::fillRect(display, 86, 140, 29, 22, GxEPD_WHITE);
          UiSDK::setFont(display, &FreeSerifBold12pt7b);
          UiSDK::setTextColor(display, GxEPD_BLACK);
          UiSDK::setCursor(display, 87, 158);
          UiSDK::print(display, wDay);

          // draw battery
          UiSDK::drawCircleHelper(display, 45, 100, 20, 2, GxEPD_WHITE);
          UiSDK::drawCircleHelper(display, 45, 100, 20, 4, GxEPD_WHITE);
          UiSDK::drawPixel(display, 65, 100, GxEPD_WHITE);
          UiSDK::drawFastVLine(display, 45, 79, 4, GxEPD_WHITE);
          UiSDK::drawFastHLine(display, 63, 100, 4, GxEPD_WHITE);
          UiSDK::drawFastVLine(display, 45, 118, 4, GxEPD_WHITE);
          UiSDK::setFont(display, &FreeSerifBoldItalic4pt7b);
          UiSDK::setTextColor(display, GxEPD_WHITE);
          UiSDK::setCursor(display, 44, 76);
          UiSDK::print(display, "1");
          UiSDK::setCursor(display, 70, 97);
          UiSDK::print(display, "1");
          UiSDK::setCursor(display, 69, 101);
          UiSDK::print(display, "--");
          UiSDK::drawPixel(display, 71, 100, GxEPD_WHITE);
          UiSDK::setCursor(display, 69, 107);
          UiSDK::print(display, "2");
          UiSDK::setCursor(display, 43, 127);
          UiSDK::print(display, "0");
          UiSDK::fillCircle(display, 45, 100, 2, GxEPD_WHITE);
          double batteryCurrent = (getBatteryVoltage() - 3.3) / 0.9;
          double batteryAngle = batteryCurrent * 180;
          double radBattery = ((batteryAngle) * 71) / 4068.0;
          double bx1 = 45 + (sin(radBattery) * 16);
          double by1 = 100 + (cos(radBattery) * 16);
          UiSDK::drawLine(display, 45, 100, (int)bx1, (int)by1, GxEPD_WHITE);

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
