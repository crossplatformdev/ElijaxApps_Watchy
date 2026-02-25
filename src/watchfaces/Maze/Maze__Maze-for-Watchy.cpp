#if defined(WATCHY_WRAPPER_INCLUDE) || defined(WATCHY_STANDALONE_WATCHFACE)
#include "../../watchy/Watchy.h" //include the Watchy library
#include "imgs.h"        

#include "../../sdk/UiSDK.h"

class MazeWatchFace : public Watchy { //inherit and extend Watchy class
  public:
    void drawWatchFace() { //override this method to customize how the watch face looks
      const unsigned char *images [10] = {img0,img1,img2,img3,img4,img5,img6,img7,img8,img9};

      UiSDK::initScreen(display);
      const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);
      const uint16_t bgColor = UiSDK::getWatchfaceBg(BASE_POLARITY);
            
      //drawbg
      UiSDK::fillScreen(display, fgColor);
      UiSDK::drawBitmap(display, 0, 0, background, 200, 200, bgColor);

      //draw time
      UiSDK::fillRect(display, 29,29,66,66, fgColor);
      UiSDK::fillRect(display, 29,107,66,66, fgColor);	
      UiSDK::fillRect(display, 107,29,66,66, fgColor);	
      UiSDK::fillRect(display, 107,107,66,66, fgColor);	
									
      UiSDK::drawBitmap(display, 29, 29, images[currentTime.Hour/10], 66, 66, bgColor);
      UiSDK::drawBitmap(display, 107, 29, images[currentTime.Hour%10], 66, 66, bgColor);
								
      UiSDK::drawBitmap(display, 29, 107, images[currentTime.Minute/10], 66, 66, bgColor);
      UiSDK::drawBitmap(display, 107, 107, images[currentTime.Minute%10], 66, 66, bgColor);
     
      // draw battery
      float batt = (getBatteryVoltage()-3.3)/0.9;
      if (batt > 0) {
       UiSDK::fillRect(display, 0,185,2,12*batt,fgColor);
      }                 
    }
};

void WatchfaceEntrypoint_Maze(Watchy &watchy) {
  MazeWatchFace face;
  face.settings = watchy.settings;
  face.currentTime = watchy.currentTime;
  face.drawWatchFace();
}

void showWatchFace_Maze(Watchy &watchy) {
  WatchfaceEntrypoint_Maze(watchy);
}

  #endif // WATCHY_WRAPPER_INCLUDE || WATCHY_STANDALONE_WATCHFACE