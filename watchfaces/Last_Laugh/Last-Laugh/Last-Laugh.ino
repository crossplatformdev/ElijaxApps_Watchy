#include "../../../src/watchy/Watchy.h"
#include "img.h"
#include "numberMaps.h"
#include "../../../src/settings/settings.h"


class LastLaugh : public Watchy {
    public:
        LastLaugh(const watchySettings& s) : Watchy(s) {}
        // Selector that draws numbers based on hours
        void selectNumber(int n, int x, int y){
          if (n == 1) {
          display.drawBitmap(x, y, fone, 6, 13, GxEPD_WHITE, GxEPD_BLACK);
          }
          if (n == 2) {
            display.drawBitmap(x, y, ftwo, 6, 13, GxEPD_WHITE, GxEPD_BLACK);
          }
          if (n == 3) {
            display.drawBitmap(x, y, fthree, 6, 13, GxEPD_WHITE, GxEPD_BLACK);
          }
          if (n == 4) {
            display.drawBitmap(x, y, ffour, 6, 13, GxEPD_WHITE, GxEPD_BLACK);
          }
          if (n == 5) {
            display.drawBitmap(x, y, ffive, 6, 13, GxEPD_WHITE, GxEPD_BLACK);
          }
          if (n == 6) {
            display.drawBitmap(x, y, fsix, 6, 13, GxEPD_WHITE, GxEPD_BLACK);
          }
          if (n == 7) {
            display.drawBitmap(x, y, fseven, 6, 13, GxEPD_WHITE, GxEPD_BLACK);
          }
          if (n == 8) {
            display.drawBitmap(x, y, feight, 6, 13, GxEPD_WHITE, GxEPD_BLACK);
          }
          if (n == 9) {
            display.drawBitmap(x, y, fnine, 6, 13, GxEPD_WHITE, GxEPD_BLACK);
          }
          if (n == 0) {
            display.drawBitmap(x, y, fnull, 6, 13, GxEPD_WHITE, GxEPD_BLACK);
          }
        }
        void drawWatchFace(){
          // Display background image
          display.drawBitmap(0, 0, laughWFWN, DISPLAY_WIDTH, DISPLAY_HEIGHT, GxEPD_WHITE, GxEPD_BLACK);
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

LastLaugh m(settings);

void setup() {
  m.init();

}

void loop() {
  // put your main code here, to run repeatedly:

}
