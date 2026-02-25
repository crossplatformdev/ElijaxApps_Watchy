#include "Watchy_Star_Wars_Aurebesh.h"
#include "../../settings/settings.h"

static WatchyStarWarsAurebesh watchy(settings);



#ifdef WATCHY_STANDALONE_WATCHFACE
void setup(){
  watchy.init();
}

void loop(){}




#endif
