#include "Watchy_Mario.h"
#include "./../../settings/settings.h"

#ifdef WATCHY_STANDALONE_WATCHFACE

static WatchyMario watchy(settings);

void setup(){
  watchy.init();
}

#endif