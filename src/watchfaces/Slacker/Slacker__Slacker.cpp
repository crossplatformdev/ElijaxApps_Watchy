#if defined(WATCHY_WRAPPER_INCLUDE) || defined(WATCHY_STANDALONE_WATCHFACE)
/*
Slacker
Version 1.2.1
https://github.com/uCBill/Slacker
Linux like watchface by Bill Eichner
Daylight Savings Time is configured in the settings.h section

Based on Bahn-for-Watchy
https://github.com/BraininaBowl/Bahn-for-Watchy

Based on SmatWatch
https://github.com/theRealc2c2/SmartWatch

Face for Watchy watch
https://watchy.sqfmi.com
*/

#include "../../watchy/Watchy.h" //include the Watchy library
#include "../../sdk/UiSDK.h"
#include "LiberationSansNarrow_Bold8pt7b.h"
#include "Teko_Regular20pt7b.h"
#include "prompt.h"

class SlackerWatchFace : public Watchy { //inherit and extend Watchy class
  using Watchy::Watchy;
  public:
    void drawWatchFace() { //override this method to customize how the watch face looks
      
      int16_t  x1, y1;
      uint16_t w, h;
      String textstring;
      int temp;

      UiSDK::initScreen(display);
      const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);
      const uint16_t bgColor = UiSDK::getWatchfaceBg(BASE_POLARITY);

      // Step counter managed by core Watchy

      //drawbg
      UiSDK::setFont(display, &LiberationSansNarrow_Bold8pt7b);
  UiSDK::setTextColor(display, fgColor);
      UiSDK::setTextWrap(display, false);
      
      //draw slacker text
      UiSDK::getTextBounds(display, textstring, 0, 0, &x1, &y1, &w, &h);
      UiSDK::setCursor(display, 0, 10);
      UiSDK::print(display, "sk@darkstar:/~");
      UiSDK::setCursor(display, 0, 23);
      UiSDK::print(display, "$ ls -l");
      UiSDK::setCursor(display, 0, 36);
      UiSDK::print(display, "total (steps)");//use step # for total
      UiSDK::setCursor(display, 0, 49);
      UiSDK::print(display, "4 drwxr-xr-x 5 sk sk 4K Jn 1 Dcs");
      UiSDK::setCursor(display, 0, 62);
      UiSDK::print(display, "4 drwxr-xr-x 2 sk sk 4K Ag 9 Dnl");
      UiSDK::setCursor(display, 0, 75);
      UiSDK::print(display, "sk@darkstar:/~");
      UiSDK::setCursor(display, 0, 88);
      UiSDK::print(display, "$ date");
      
      // draw day of week
      textstring = dayShortStr(currentTime.Wday);
      textstring += " ";

      //draw day, month, year
      textstring += currentTime.Day;
      textstring += " ";
      textstring += monthShortStr(currentTime.Month);
      textstring += " ";
      textstring += currentTime.Year + 1970;
      textstring += " ";
      
      
      //draw time
      UiSDK::setFont(display, &LiberationSansNarrow_Bold8pt7b);
     if (currentTime.Hour > 0 && currentTime.Hour < 10) {
        textstring += "0";
      } else if(currentTime.Hour > 12 && currentTime.Hour <= 21){
        textstring += "0";
      } else {
        textstring += "";
      }
      if (currentTime.Hour > 0 && currentTime.Hour <= 12) {
        textstring += currentTime.Hour;
      } else if (currentTime.Hour < 1) {
        textstring += 12;
      } else {
        textstring += ((currentTime.Hour+11)%12)+1;
      }

      textstring += ":";
      if (currentTime.Minute < 10) {
        textstring += "0";
      } else {
        textstring += "";
      }
      textstring += currentTime.Minute;

      //this section adds AM or PM to the display
      if (currentTime.Hour >= 12) {
        textstring += " PM EDT";//List of US Time Zones: EST, CST, MST, PST, AKST
      } else {
        textstring += " AM EDT";//List of US Daylight Savings Time Zones: EDT, CDT, MDT, PDT, AKDT
	  }
   //To change Time Zones, including Daylight Savings Time, see the settings.h section

      UiSDK::setCursor(display, 0, 102);
      UiSDK::print(display, textstring);

      //drawTimeBold
      UiSDK::setFont(display, &Teko_Regular20pt7b);
      if (currentTime.Hour > 0 && currentTime.Hour <= 12) {
        textstring = currentTime.Hour;
      } else if (currentTime.Hour < 1) {
        textstring = 12;
      } else {
        textstring = ((currentTime.Hour+11)%12)+1;
      }

      textstring += ":";
      if (currentTime.Minute < 10) {
        textstring += "0";
      } else {
        textstring += "";
      }
      textstring += currentTime.Minute;

      //this section adds AM or PM to the display
      if (currentTime.Hour >= 12) {
        textstring += "PM";
      } else {
        textstring += "AM";
    }

      UiSDK::setCursor(display, 107, 23);
      UiSDK::print(display, textstring);
      

      
      // draw battery
      UiSDK::setFont(display, &LiberationSansNarrow_Bold8pt7b);
     int8_t batteryLevel = 0;
     float VBAT = getBatteryVoltage();

     if(VBAT >= 4.2){
        batteryLevel = 100.0;
     }
     else if (VBAT >= 3.3) {
        batteryLevel = 100.0*(VBAT-3.3)/0.9;
    }
      UiSDK::getTextBounds(display, textstring, 0, 0, &x1, &y1, &w, &h);
      UiSDK::setCursor(display, 0, 116);
      UiSDK::print(display, "sk@darkstar:/~");
      UiSDK::setCursor(display, 0, 129);
      UiSDK::print(display, "$ upower -i/UPwr/dvcs/btry_BT0");
      UiSDK::setCursor(display, 0, 143);
      UiSDK::print(display, "voltage:");
      UiSDK::setCursor(display, 120, 143);
      UiSDK::print(display, VBAT);
      UiSDK::setCursor(display, 0, 157);
      UiSDK::print(display, "percentage:");
      UiSDK::setCursor(display, 120, 157);
      UiSDK::print(display, batteryLevel); 
      UiSDK::setCursor(display, 143, 157);
      UiSDK::print(display, "%"); 
      UiSDK::setCursor(display, 0, 171);
      UiSDK::print(display, "technology:");
      UiSDK::setCursor(display, 120, 171);
      UiSDK::print(display, "lithium-pol");
      UiSDK::setCursor(display, 0, 185);
      UiSDK::print(display, "sk@darkstar:/~");
      UiSDK::setCursor(display, 0, 199);
      UiSDK::print(display, "$ ");
      //draw image
      UiSDK::drawBitmap(display, 12,187, epd_bitmap_prompt, 7, 14, fgColor);


      //draw steps
      UiSDK::setFont(display, &LiberationSansNarrow_Bold8pt7b);
      textstring = sensor.getCounter();
      UiSDK::getTextBounds(display, textstring, 0, 0, &x1, &y1, &w, &h);
      UiSDK::setCursor(display, 75, 36);
      UiSDK::setTextColor(display, fgColor);
      UiSDK::print(display, textstring);


    }

};

void WatchfaceEntrypoint_Slacker(Watchy &watchy) {
  SlackerWatchFace face;
  face.settings = watchy.settings;
  face.currentTime = watchy.currentTime;
  face.drawWatchFace();
}

void showWatchFace_Slacker(Watchy &watchy) {
  WatchfaceEntrypoint_Slacker(watchy);
}

#endif // WATCHY_WRAPPER_INCLUDE || WATCHY_STANDALONE_WATCHFACE


