#ifndef WATCHY_WRAPPER_INCLUDE

#include "Watchy_Tetris.h"
#include "settings.h"

WatchyTetris watchy(settings);

void setup(){
  watchy.init();
}

void loop(){}

#endif
