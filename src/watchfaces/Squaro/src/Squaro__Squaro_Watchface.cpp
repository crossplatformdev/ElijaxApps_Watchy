#include "Watchy_Squaro.h"
#include "../../../settings/settings.h"

static Squaro_Watchface watchy(settings);



#ifdef WATCHY_STANDALONE_WATCHFACE
void setup(){
  watchy.init();
}

void loop(){}

#endif
