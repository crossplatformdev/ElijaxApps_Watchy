#if defined(WATCHY_WRAPPER_INCLUDE) || defined(WATCHY_STANDALONE_WATCHFACE)
#include "../../../watchy/Watchy.h"
#include "../../../sdk/UiSDK.h"
#include "img.h"
#include "numberMaps.h"


class LastLaugh : public Watchy {
    public:
        LastLaugh(const watchySettings& s) : Watchy(s) {}
        // Selector that draws numbers based on hours
        void selectNumber(int n, int x, int y){
          const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);
          const uint16_t bgColor = UiSDK::getWatchfaceBg(BASE_POLARITY);
          if (n == 1) {
          UiSDK::drawBitmap(display, x, y, fone, 6, 13, fgColor, bgColor);
          }
          if (n == 2) {
            UiSDK::drawBitmap(display, x, y, ftwo, 6, 13, fgColor, bgColor);
          }
          if (n == 3) {
            UiSDK::drawBitmap(display, x, y, fthree, 6, 13, fgColor, bgColor);
          }
          if (n == 4) {
            UiSDK::drawBitmap(display, x, y, ffour, 6, 13, fgColor, bgColor);
          }
          if (n == 5) {
            UiSDK::drawBitmap(display, x, y, ffive, 6, 13, fgColor, bgColor);
          }
          if (n == 6) {
            UiSDK::drawBitmap(display, x, y, fsix, 6, 13, fgColor, bgColor);
          }
          if (n == 7) {
            UiSDK::drawBitmap(display, x, y, fseven, 6, 13, fgColor, bgColor);
          }
          if (n == 8) {
            UiSDK::drawBitmap(display, x, y, feight, 6, 13, fgColor, bgColor);
          }
          if (n == 9) {
            UiSDK::drawBitmap(display, x, y, fnine, 6, 13, fgColor, bgColor);
          }
          if (n == 0) {
            UiSDK::drawBitmap(display, x, y, fnull, 6, 13, fgColor, bgColor);
          }
        }
        void drawWatchFace(){
          UiSDK::initScreen(display);
          const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);
          const uint16_t bgColor = UiSDK::getWatchfaceBg(BASE_POLARITY);

          // Display background image
          UiSDK::drawBitmap(display, 0, 0, laughWFWN, DISPLAY_WIDTH, DISPLAY_HEIGHT, fgColor, bgColor);
          if (currentTime.Hour < 10){
            selectNumber(currentTime.Hour, 102, 133);
          }
          if (currentTime.Hour >= 10){
            selectNumber((currentTime.Hour / 1U) % 10, 102, 133);
            selectNumber((currentTime.Hour / 10U) % 10, 85, 133);
          }
          if (currentTime.Minute < 10){
            selectNumber(currentTime.Minute, 104, 170);
          }
          if (currentTime.Minute >= 10){
            selectNumber((currentTime.Minute / 1U) % 10, 104, 170);
            selectNumber((currentTime.Minute / 10U) % 10, 92, 170);
          }
          
        }
};

void WatchfaceEntrypoint_LastLaugh(Watchy &watchy) {
  LastLaugh face(watchy.settings);
  face.currentTime = watchy.currentTime;
  face.drawWatchFace();
}

void showWatchFace_Last_Laugh(Watchy &watchy) {
  WatchfaceEntrypoint_LastLaugh(watchy);
}

#ifdef WATCHY_STANDALONE_WATCHFACE
watchySettings settings;
LastLaugh m(settings);

void setup() {
  m.init();
}

void loop() {
  // this should never run, Watchy deep sleeps after init();
}
#endif

#endif // WATCHY_WRAPPER_INCLUDE || WATCHY_STANDALONE_WATCHFACE