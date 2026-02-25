#include "erikaType.h"
#include "../../settings/settings.h"

static erikaType watchy(settings);



#ifdef WATCHY_STANDALONE_WATCHFACE
void setup(){
  watchy.init();
}

void loop(){}

#endif
