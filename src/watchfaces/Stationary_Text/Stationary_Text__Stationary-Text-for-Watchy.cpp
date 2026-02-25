#if defined(WATCHY_WRAPPER_INCLUDE) || defined(WATCHY_STANDALONE_WATCHFACE)
#include "../../watchy/Watchy.h" //include the Watchy library
#include "NunitoSans_Bold28pt7b.h"
#include "NunitoSans_Light28pt7b.h"

#include "../../sdk/UiSDK.h"
        

class StationaryTextWatchFace : public Watchy { //inherit and extend Watchy class
  public:
    void drawWatchFace() { //override this method to customize how the watch face looks
      uint16_t lines = 0;
      const char *lows [10] = {"zero", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine"};
      const char *teensone [11] = {"","ten", "eleven", "twelve", "thir", "four", "fif", "six", "seven", "eight", "nine"};
      const char *teenstwo [11] = {"", "","", "teen", "teen", "teen", "teen", "teen", "teen", "teen", "teen"};
      const char *tens [10] = {"zero", "ten", "twenty", "thirty", "forty", "fifty", "sixty", "seventy", "eighty", "ninety"};

      // draw bg
      UiSDK::initScreen(display);
      const uint16_t bgColor = UiSDK::getWatchfaceBg(BASE_POLARITY);
      const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);

      UiSDK::setTextColor(display, fgColor);
      UiSDK::setTextWrap(display, false);

      //drawtime
      lines += 1;
      UiSDK::setCursor(display, 8, lines*47-5);
      UiSDK::setFont(display, &NunitoSans_Bold28pt7b);
      if (currentTime.Hour == 0) {
        UiSDK::print(display, tens[2]);
        lines += 1;
        UiSDK::setCursor(display, 8, lines*47-5);
        UiSDK::print(display, lows[4]);
      } else if (currentTime.Hour < 10) {
        UiSDK::print(display, lows[currentTime.Hour]);
      } else if (currentTime.Hour < 20) {
        UiSDK::print(display, teensone[currentTime.Hour-9]);
        if (currentTime.Hour > 12) {
          lines += 1;
          UiSDK::setCursor(display, 8, lines*47-5);
          UiSDK::print(display, teenstwo[currentTime.Hour%10]);
        }
      } else {
        UiSDK::print(display, tens[currentTime.Hour/10]);
        if (currentTime.Hour%10 > 0) {
          lines += 1;
          UiSDK::setCursor(display, 8, lines*47-5);
          UiSDK::print(display, lows[currentTime.Hour%10]);
        }
      }

      lines += 1;
      UiSDK::setCursor(display, 8, lines*47-5);
      UiSDK::setFont(display, &NunitoSans_Light28pt7b);
      if (currentTime.Minute == 0) {
        UiSDK::print(display, "o'clock");
      } else if (currentTime.Minute < 10) {
          UiSDK::print(display, "oh");
          lines += 1;
          UiSDK::setCursor(display, 8, lines*47-5);
          UiSDK::print(display, lows[currentTime.Minute]);
      } else if (currentTime.Minute < 20) {
        UiSDK::print(display, teensone[currentTime.Minute-9]);
        if (currentTime.Minute > 12) {
          lines += 1;
          UiSDK::setCursor(display, 8, lines*47-5);
          UiSDK::print(display, teenstwo[currentTime.Minute%10]);
        }
      } else {
        UiSDK::println(display, tens[currentTime.Minute/10]);
        if (currentTime.Minute%10 > 0) {
          lines += 1;
          UiSDK::setCursor(display, 8, lines*47-5);
          UiSDK::print(display, lows[currentTime.Minute%10]);
        }
      }                
    }
};

void WatchfaceEntrypoint_StationaryText(Watchy &watchy) {
  StationaryTextWatchFace face;
  face.settings = watchy.settings;
  face.currentTime = watchy.currentTime;
  face.drawWatchFace();
}

void showWatchFace_Stationary_Text(Watchy &watchy) {
  WatchfaceEntrypoint_StationaryText(watchy);
}

  #endif // WATCHY_WRAPPER_INCLUDE || WATCHY_STANDALONE_WATCHFACE

