#include "Watchy_Pokemon.h"
#include "pokemon.h"
#include "sdk/WatchyUi.h"

void WatchyPokemon::drawWatchFace(){
    const uint16_t foreground = WatchyUi::Theme::foreground();
    display.drawBitmap(0, 0, pokemon, DISPLAY_WIDTH, DISPLAY_HEIGHT, foreground);
    display.setTextColor(foreground);
    display.setFont(&FreeMonoBold9pt7b);
    display.setCursor(10, 170);
    if(currentTime.Hour < 10){
        display.print('0');
    }
    display.print(currentTime.Hour);
    display.print(':');
    if(currentTime.Minute < 10){
        display.print('0');
    }
    display.print(currentTime.Minute);
}