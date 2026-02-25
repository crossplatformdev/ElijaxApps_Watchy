#if defined(WATCHY_WRAPPER_INCLUDE) || defined(WATCHY_STANDALONE_WATCHFACE)
#include "../../watchy/Watchy.h" //include the Watchy library
#include "soloist115pt7b.h" //include any fonts you want to use
#include "backTwo.h"   //Image Background
#include "res.h"       //Second Image

#include "../../sdk/UiSDK.h"

class SWWatchyFace : public Watchy{ //inherit and extend Watchy class
    public:
        void drawWatchFace(){ //override this method to customize how the watch face looks
          UiSDK::initScreen(display);
          const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);
          UiSDK::setFont(display, &soloist115pt7b);
          UiSDK::setTextColor(display, fgColor);

          //Image
          if(currentTime.Minute >= 30){
            UiSDK::drawBitmap(display, 0, 0, resHalfresisTest, DISPLAY_WIDTH, DISPLAY_HEIGHT, fgColor);
          }else{
            UiSDK::drawBitmap(display, 0, 0, backbackgroundTwo, DISPLAY_WIDTH, DISPLAY_HEIGHT, fgColor);
          }

          //HourTime
          UiSDK::drawRoundRect(display, 78, 17, 103, 38, 10, fgColor);
          UiSDK::setCursor(display, 78, 45);
          if(currentTime.Hour < 10){ //use the currentTime struct to print latest time
            UiSDK::print(display, "0");
          }
          UiSDK::print(display, currentTime.Hour);
          UiSDK::print(display, ":");
          if(currentTime.Minute < 10){
            UiSDK::print(display, "0");
          }  
          UiSDK::println(display, currentTime.Minute);

          //Battery
            UiSDK::drawRoundRect(display, 187, 10, 5, 180, 10, fgColor);
          float VBAT = getBatteryVoltage();
          if(VBAT > 4.1){
              UiSDK::fillRoundRect(display, 187, 10, 5, 180, 10, fgColor);
          }
          else if(VBAT > 3.95 && VBAT <= 4.1){
              UiSDK::fillRoundRect(display, 187, 10, 5, 120, 10, fgColor);
          }
          else if(VBAT > 3.80 && VBAT <= 3.95){
              UiSDK::fillRoundRect(display, 187, 10, 5, 90, 10, fgColor);
          }    
          else if(VBAT <= 3.80){
              UiSDK::fillRoundRect(display, 187, 10, 5, 30, 10, fgColor);
          }
        }                
};

void WatchfaceEntrypoint_SW_Watchy(Watchy &watchy) {
  SWWatchyFace face;
  face.settings = watchy.settings;
  face.currentTime = watchy.currentTime;
  face.drawWatchFace();
}

void showWatchFace_SW_Watchy(Watchy &watchy) {
  WatchfaceEntrypoint_SW_Watchy(watchy);
}

#ifdef WATCHY_STANDALONE_WATCHFACE
SWWatchyFace m; //instantiate your watchface

void setup() {
  m.init(); //call init in setup
}

void loop() {
  // this should never run, Watchy deep sleeps after init();
}
#endif

#endif // WATCHY_WRAPPER_INCLUDE || WATCHY_STANDALONE_WATCHFACE