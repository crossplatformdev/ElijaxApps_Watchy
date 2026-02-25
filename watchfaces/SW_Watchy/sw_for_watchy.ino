#include "../../src/watchy/Watchy.h" //include the Watchy library
#include "soloist115pt7b.h" //include any fonts you want to use
#include "backTwo.h"   //Image Background
#include "res.h"       //Second Image

class MyFirstWatchFace : public Watchy{ //inherit and extend Watchy class
    public:
        void drawWatchFace(){ //override this method to customize how the watch face looks
          display.setFont(&soloist115pt7b);

          //Image
          if(currentTime.Minute >= 30){
            display.fillScreen(GxEPD_BLACK);
            display.drawBitmap(0, 0, resHalfresisTest, DISPLAY_WIDTH, DISPLAY_HEIGHT, GxEPD_WHITE);
          }else{
            display.fillScreen(GxEPD_BLACK);
            display.drawBitmap(0, 0, backbackgroundTwo, DISPLAY_WIDTH, DISPLAY_HEIGHT, GxEPD_WHITE);
          }

          //HourTime
          display.drawRoundRect(78, 17, 103, 38, 10, GxEPD_WHITE);
          display.setCursor(78, 45);
          if(currentTime.Hour < 10){ //use the currentTime struct to print latest time
            display.print("0");
          }
          display.print(currentTime.Hour);
          display.print(":");
          if(currentTime.Minute < 10){
            display.print("0");
          }  
          display.println(currentTime.Minute);

          //Battery
          display.drawRoundRect(187, 10, 5, 180, 10, GxEPD_WHITE);
          float VBAT = getBatteryVoltage();
          if(VBAT > 4.1){
              display.fillRoundRect(187, 10, 5, 180, 10, GxEPD_WHITE);
          }
          else if(VBAT > 3.95 && VBAT <= 4.1){
              display.fillRoundRect(187, 10, 5, 120, 10, GxEPD_WHITE);
          }
          else if(VBAT > 3.80 && VBAT <= 3.95){
              display.fillRoundRect(187, 10, 5, 90, 10, GxEPD_WHITE);
          }    
          else if(VBAT <= 3.80){
              display.fillRoundRect(187, 10, 5, 30, 10, GxEPD_WHITE);
          }
        }                
};

MyFirstWatchFace m; //instantiate your watchface

void setup() {
  m.init(); //call init in setup
}

void loop() {
  // this should never run, Watchy deep sleeps after init();
}
