#include "Watchy_Tetris.h"
#include "tetris.h"

namespace {

const unsigned char *const tetrisNums[10] = {
    tetris0, tetris1, tetris2, tetris3, tetris4,
    tetris5, tetris6, tetris7, tetris8, tetris9};

} // namespace

void WatchyTetris::drawWatchFace(Watchy &watch) {
  Watchy::display.fillScreen(GxEPD_WHITE);
  Watchy::display.drawBitmap(0, 0, tetrisbg, DISPLAY_WIDTH, DISPLAY_HEIGHT,
                     GxEPD_BLACK);

  Watchy::display.drawBitmap(25, 20, tetrisNums[watch.currentTime.Hour / 10], 40, 60,
                     GxEPD_BLACK);
  Watchy::display.drawBitmap(75, 20, tetrisNums[watch.currentTime.Hour % 10], 40, 60,
                     GxEPD_BLACK);
  Watchy::display.drawBitmap(25, 110, tetrisNums[watch.currentTime.Minute / 10], 40, 60,
                     GxEPD_BLACK);
  Watchy::display.drawBitmap(75, 110, tetrisNums[watch.currentTime.Minute % 10], 40, 60,
                     GxEPD_BLACK);
}

#ifndef WATCHY_WRAPPER_INCLUDE

#include "settings.h"

void setup(){
  WatchySdk::settings = settings;
  WatchySdk::init();
}

void loop(){}

#endif
