#include "Watchy_Mario.h"
#include "mario.h"

namespace {

constexpr uint8_t numW = 44;
constexpr uint8_t numH = 44;
constexpr uint8_t coinW = 24;
constexpr uint8_t coinH = 30;
constexpr uint8_t pipeW = 42;
constexpr uint8_t pipeH = 47;
constexpr uint8_t marioW = 56;
constexpr uint8_t marioH = 54;
constexpr uint8_t numSpacing = 4;
constexpr uint8_t coinSpacing = 4;
constexpr uint8_t floorH = 19;
constexpr int16_t pipePadding = DISPLAY_HEIGHT - floorH - pipeH;
constexpr int16_t xPadding =
    (DISPLAY_WIDTH - (4 * numW) - (3 * numSpacing)) / 2;
constexpr int16_t yPadding = 2 * coinSpacing + coinH;

const unsigned char *const numbers[10] = {
    mario0, mario1, mario2, mario3, mario4,
    mario5, mario6, mario7, mario8, mario9};

} // namespace

void WatchyMario::drawWatchFace(Watchy &watch) {
  Watchy::display.fillScreen(GxEPD_WHITE);
  Watchy::display.drawBitmap(0, 0, mariobg, DISPLAY_WIDTH, DISPLAY_HEIGHT,
                     GxEPD_BLACK);

  int hour10 = watch.currentTime.Hour / 10;
  int hour01 = watch.currentTime.Hour % 10;
  int minute10 = watch.currentTime.Minute / 10;
  int minute01 = watch.currentTime.Minute % 10;
  int pos = 3;

  if (hour01 == 0 && minute10 == 0 && minute01 == 0) {
    pos = 0;
  } else if (minute10 == 0 && minute01 == 0) {
    pos = 1;
  } else if (minute01 == 0) {
    pos = 2;
  }

  int16_t digitX = xPadding + pos * (numSpacing + numW);
  Watchy::display.drawBitmap(digitX + (numW / 2 - marioW / 2) +
                         (pos < 2 ? 8 : -8),
                     yPadding + numH + 4,
                     pos < 2 ? mariomariol : mariomarior,
                     marioW, marioH, GxEPD_BLACK);
  Watchy::display.drawBitmap(digitX + (numW / 2 - coinW / 2), coinSpacing,
                     mariocoin, coinW, coinH, GxEPD_BLACK);

  if (pos == 0) {
    Watchy::display.drawBitmap(DISPLAY_WIDTH - 2 * pipeW, pipePadding, mariopipe,
                       pipeW, pipeH, GxEPD_BLACK);
  } else if (pos == 1 || pos == 2) {
    Watchy::display.drawBitmap(xPadding, pipePadding, mariopipe, pipeW, pipeH,
                       GxEPD_BLACK);
    Watchy::display.drawBitmap(DISPLAY_WIDTH - pipeW - xPadding, pipePadding,
                       mariopipe, pipeW, pipeH, GxEPD_BLACK);
  } else {
    Watchy::display.drawBitmap(2 * pipeW, pipePadding, mariopipe, pipeW, pipeH,
                       GxEPD_BLACK);
  }

  Watchy::display.drawBitmap(xPadding, pos == 0 ? yPadding : yPadding + 20,
                     numbers[hour10], numW, numH, GxEPD_BLACK);
  Watchy::display.drawBitmap(xPadding + numSpacing + numW,
                     pos == 1 ? yPadding : yPadding + 20,
                     numbers[hour01], numW, numH, GxEPD_BLACK);
  Watchy::display.drawBitmap(xPadding + 2 * (numSpacing + numW),
                     pos == 2 ? yPadding : yPadding + 20,
                     numbers[minute10], numW, numH, GxEPD_BLACK);
  Watchy::display.drawBitmap(xPadding + 3 * (numSpacing + numW),
                     pos == 3 ? yPadding : yPadding + 20,
                     numbers[minute01], numW, numH, GxEPD_BLACK);
}

#ifndef WATCHY_WRAPPER_INCLUDE

#include "settings.h"

void setup(){
  WatchySdk::settings = settings;
  WatchySdk::init();
}

void loop(){}

#endif