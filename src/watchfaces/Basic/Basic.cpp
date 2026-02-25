#ifndef WATCHY_WRAPPER_INCLUDE

#include <Watchy.h>
#include "settings.h"

Watchy watchy(settings);

void setup(){
  watchy.init();
}

void loop(){}

#endif