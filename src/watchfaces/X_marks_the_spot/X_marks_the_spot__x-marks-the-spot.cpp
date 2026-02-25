#include "x.h"
#include "../../settings/settings.h"

static X watchy(settings);



#ifdef WATCHY_STANDALONE_WATCHFACE
void setup(){
  watchy.init();
}

void loop(){}
#endif
