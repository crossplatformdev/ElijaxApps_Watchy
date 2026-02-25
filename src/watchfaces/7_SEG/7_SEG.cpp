#ifndef WATCHY_WRAPPER_INCLUDE

#include "fonts/Watchy_7_SEG.h"
#include "settings.h"

void setup(){
  WatchySdk::settings = settings;
  WatchySdk::init();
}

void loop(){}


#endif



