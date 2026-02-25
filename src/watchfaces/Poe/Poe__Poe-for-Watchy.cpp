#if defined(WATCHY_WRAPPER_INCLUDE) || defined(WATCHY_STANDALONE_WATCHFACE)
#include "../../watchy/Watchy.h" //include the Watchy library
#include "../../sdk/UiSDK.h"
#include "PTSerif_Bold10pt7b.h"
#include "PTSerif_Regular10pt7b.h"
#include "raven.h"        

class PoeWatchFace : public Watchy { //inherit and extend Watchy class
  public:

    // Global vars
    int16_t lineheight = 24;
    int16_t curx = 68;
    int16_t cury = 12 + lineheight;

    // Text drawing
    void drawText(String text){
      int16_t  x1, y1, testx, counter;
      uint16_t w, h;

      UiSDK::getTextBounds(display, text, 0, 0, &x1, &y1, &w, &h);
      testx = curx + w;
      if (testx > 194){
        cury += lineheight; 
        if (cury < 64) {
          curx = 68;
        } else {
          curx = 6;
        }
      }
      UiSDK::setCursor(display, curx,cury);
      curx += w + 5;
      UiSDK::print(display, text);
    }
  
    void drawWatchFace() { //override this method to customize how the watch face looks

      UiSDK::initScreen(display);
      const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);

      int16_t hoffset;
      int16_t arraylength[] = {3,0,27};
      String nmbrs[] = {"midnight", "one", "two", "three", "four", "five", "six", "seven", "eight","nine","ten","eleven","twelve","thirteen","fourteen","quarter","sixteen","seventeen","eighteen","nineteen","twenty","twenty-one","twenty-two", "twenty-three", "twenty-four", "twenty-five", "twenty-six", "twenty-seven", "twenty-eight", "twenty-nine", "half" };
      String pretext[] = {"nce", "upon", "a"};
      String text[10] = {};
      if (currentTime.Hour == 12 && currentTime.Minute == 0){
        text[arraylength[1]] = "noon";
        arraylength[1] += 1; 
      } else {
        if (currentTime.Minute == 0){
        } else if (currentTime.Minute < 31) {
          text[arraylength[1]] = nmbrs[currentTime.Minute];
          arraylength[1] += 1; 
          text[arraylength[1]] = "past";
          arraylength[1] += 1;
          hoffset = currentTime.Hour;
        } else {
          text[arraylength[1]] = nmbrs[60-currentTime.Minute];
          arraylength[1] += 1; 
          text[arraylength[1]] = "to";
          arraylength[1] += 1;           
          hoffset = currentTime.Hour + 1;
        }
        if (hoffset < 13){
          text[arraylength[1]] = nmbrs[hoffset];
          arraylength[1] += 1;       
        } else if (hoffset == 13) {  
          text[arraylength[1]] = "one";
          arraylength[1] += 1; 
        } else {
          text[arraylength[1]] = nmbrs[hoffset-12];
          arraylength[1] += 1; 
        }
        if (currentTime.Minute==0 && currentTime.Hour != 0 && currentTime.Hour != 12) {
          text[arraylength[1]] = "o'clock";
          arraylength[1] += 1;
        }
      }
      
      String posttext[] = {"dreary,", "while", "I", "pondered,", "weak", "and", "weary,", "over", "many", "a", "quaint", "and", "curious", "volume", "of", "forgotten", "lore.", "While", "I", "nodded,", "nearly", "napping,", "suddenly", "there", "came", "a", "tapping"};
      
      //drawbg
      UiSDK::drawBitmap(display, 4, 4, Raven, 60, 60, fgColor);

      //draw time
      UiSDK::setTextColor(display, fgColor);
      UiSDK::setTextWrap(display, false);

      UiSDK::setFont(display, &PTSerif_Regular10pt7b);
      for (int i=0; i<arraylength[0]; i++) {
        drawText(pretext[i]);
      }

      UiSDK::setFont(display, &PTSerif_Bold10pt7b);
      for (int i=0; i<arraylength[1]; i++) {
        drawText(text[i]);
      }

      UiSDK::setFont(display, &PTSerif_Regular10pt7b);
      for (int i=0; i<arraylength[2]; i++) {
        drawText(posttext[i]);
      }

    
    }
};

void WatchfaceEntrypoint_Poe(Watchy &watchy) {
  PoeWatchFace face;
  face.settings = watchy.settings;
  face.currentTime = watchy.currentTime;
  face.drawWatchFace();
}

void showWatchFace_Poe(Watchy &watchy) {
  WatchfaceEntrypoint_Poe(watchy);
}

  #endif // WATCHY_WRAPPER_INCLUDE || WATCHY_STANDALONE_WATCHFACE

