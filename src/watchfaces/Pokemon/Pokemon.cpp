#ifndef WATCHY_WRAPPER_INCLUDE

#include "Watchy_Pokemon.h"
#include "settings.h"

WatchyPokemon watchy(settings);

void setup(){
  watchy.init();
}

void loop(){}

#endif
