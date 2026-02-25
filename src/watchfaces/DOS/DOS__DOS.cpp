#include "Watchy_DOS.h"
#include "../../settings/settings.h"

static WatchyDOS watchy(settings);



#ifdef WATCHY_STANDALONE_WATCHFACE
void setup(){
  watchy.init();
}

void loop(){}

#endif
