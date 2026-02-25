#include "Watchy_Pokemon.h"
#include "../../settings/settings.h"

static WatchyPokemon watchy(settings);



#ifdef WATCHY_STANDALONE_WATCHFACE
void setup(){
  watchy.init();
}

void loop(){}

#endif
