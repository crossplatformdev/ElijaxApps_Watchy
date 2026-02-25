#include "Watchy_Squarbital.h"
#include "../../../settings/settings.h"

static Squarbital_Watchface watchy(settings);



#ifdef WATCHY_STANDALONE_WATCHFACE
void setup(){
  watchy.init();
}

void loop(){}

#endif
