#ifndef WATCHY_WRAPPER_INCLUDE

#include "Watchy_DOS.h"
#include "settings.h"

WatchyDOS watchy(settings);

void setup(){
  watchy.init();
}

void loop(){}

#endif
