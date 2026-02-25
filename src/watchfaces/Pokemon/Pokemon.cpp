#include "Watchy_Pokemon.h"
#include "pokemon.h"

void WatchyPokemon::drawWatchFace(Watchy &watch) {
  Watchy::display.fillScreen(GxEPD_WHITE);
  Watchy::display.drawBitmap(0, 0, pokemon, DISPLAY_WIDTH, DISPLAY_HEIGHT,
                     GxEPD_BLACK);
  Watchy::display.setTextColor(GxEPD_BLACK);
  Watchy::display.setFont(&FreeMonoBold9pt7b);
  Watchy::display.setCursor(10, 170);
  if (watch.currentTime.Hour < 10) {
    Watchy::display.print('0');
  }
  Watchy::display.print(watch.currentTime.Hour);
  Watchy::display.print(':');
  if (watch.currentTime.Minute < 10) {
    Watchy::display.print('0');
  }
  Watchy::display.print(watch.currentTime.Minute);
}

#ifndef WATCHY_WRAPPER_INCLUDE

#include "settings.h"

void setup(){
  WatchySdk::settings = settings;
  WatchySdk::init();
}

void loop(){}

#endif
