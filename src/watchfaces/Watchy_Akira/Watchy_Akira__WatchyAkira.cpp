#if defined(WATCHY_WRAPPER_INCLUDE) || defined(WATCHY_STANDALONE_WATCHFACE)
#include "../../watchy/Watchy.h"          //include the Watchy library
#include "../../sdk/UiSDK.h"
#include "soloist115pt7b.h"  //include any fonts you want to use
#include "Orbitron_Bold_16.h"
#include "profilebike.h"             //Akira bike image

class AkiraFace : public Watchy {  //inherit and extend Watchy class
public:
  AkiraFace(const watchySettings& s)
    : Watchy(s) {}
  void drawWatchFace() {  //override this method to customize how the watch face looks

    UiSDK::initScreen(display);
    const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);
    const uint16_t bgColor = UiSDK::getWatchfaceBg(BASE_POLARITY);

    UiSDK::drawBitmap(display, 0, 0, profilebike, DISPLAY_WIDTH, DISPLAY_HEIGHT, fgColor);


    //Day of the week
    // Authored as bg text over fg bitmap.
    UiSDK::setTextColor(display, bgColor);
    UiSDK::setFont(display, &Orbitron_Bold_16);
    UiSDK::setCursor(display, 15, 30);
    UiSDK::println(display, dayStr(currentTime.Wday));

    //Day of the month
    UiSDK::setCursor(display, 15, 55);
    UiSDK::println(display, monthShortStr(currentTime.Month));

    UiSDK::setCursor(display, 55, 55);
    UiSDK::println(display, currentTime.Day);



    //Display Hour
    UiSDK::setFont(display, &soloist115pt7b);
    UiSDK::setCursor(display, 100, 60);
    if (currentTime.Hour < 10) {  //use the currentTime struct to print latest time
      UiSDK::print(display, "0");
    }
    UiSDK::print(display, currentTime.Hour);
    UiSDK::print(display, ":");
    if (currentTime.Minute < 10) {
      UiSDK::print(display, "0");
    }
    UiSDK::println(display, currentTime.Minute);


  }
};

void WatchfaceEntrypoint_WatchyAkira(Watchy &watchy) {
  AkiraFace face(watchy.settings);
  face.currentTime = watchy.currentTime;
  face.drawWatchFace();
}

void showWatchFace_Watchy_Akira(Watchy &watchy) {
  WatchfaceEntrypoint_WatchyAkira(watchy);
}

#ifdef WATCHY_STANDALONE_WATCHFACE
watchySettings settings;
AkiraFace m(settings);  //instantiate your watchface

void setup() {
  m.init();  //call init in setup
}

void loop() {
  // this should never run, Watchy deep sleeps after init();
}
#endif

#endif // WATCHY_WRAPPER_INCLUDE || WATCHY_STANDALONE_WATCHFACE