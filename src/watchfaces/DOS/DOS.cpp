#ifndef WATCHY_WRAPPER_INCLUDE

#include "Watchy_DOS.h"
#include "settings.h"

void setup(){
  WatchySdk::settings = settings;
  WatchySdk::init();
}

void loop(){}

#endif
