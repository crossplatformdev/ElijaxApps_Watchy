// Orbital for Watchy by Sudrien and Cqoicebordel
// Released under free MIT License : https://github.com/sudrien/watchy_orbital/blob/main/LICENSE

#include "watchy_orbital.h"
#include "../../../settings/settings.h"

static WatchyOrbital watchy(settings);



#ifdef WATCHY_STANDALONE_WATCHFACE
void setup(){
  watchy.init();
}

void loop(){}

#endif
