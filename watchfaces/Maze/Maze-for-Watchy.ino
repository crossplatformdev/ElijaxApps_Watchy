#include "../../src/watchy/Watchy.h" //include the Watchy library
#include "imgs.h"        

class WatchFace : public Watchy { //inherit and extend Watchy class
  public:
    void drawWatchFace() { //override this method to customize how the watch face looks
      const unsigned char *images [10] = {img0,img1,img2,img3,img4,img5,img6,img7,img8,img9};
            
      //drawbg
      display.fillScreen(GxEPD_BLACK);
      display.drawBitmap(0, 0, background, 200, 200, GxEPD_WHITE);

      //draw time
			display.fillRect(29,29,66,66, GxEPD_BLACK);
			display.fillRect(29,107,66,66, GxEPD_BLACK);	
			display.fillRect(107,29,66,66, GxEPD_BLACK);	
			display.fillRect(107,107,66,66, GxEPD_BLACK);	
									
			display.drawBitmap(29, 29, images[currentTime.Hour/10], 66, 66, GxEPD_WHITE);
      display.drawBitmap(107, 29, images[currentTime.Hour%10], 66, 66, GxEPD_WHITE);
								
			display.drawBitmap(29, 107, images[currentTime.Minute/10], 66, 66, GxEPD_WHITE);
      display.drawBitmap(107, 107, images[currentTime.Minute%10], 66, 66, GxEPD_WHITE);
     
      // draw battery
      float batt = (getBatteryVoltage()-3.3)/0.9;
      if (batt > 0) {
       display.fillRect(0,185,2,12*batt,GxEPD_BLACK);
      }                 
    }
};

WatchFace m; //instantiate your watchface

void setup() {
  m.init(); //call init in setup
}

void loop() {
  // this should never run, Watchy deep sleeps after init();
}
