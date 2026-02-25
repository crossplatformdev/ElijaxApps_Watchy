#include "MarqueeWatch.h"
#include "../../../settings/settings.h"

static MarqueeWatch watchy(settings);



#ifdef WATCHY_STANDALONE_WATCHFACE
void setup() {
  watchy.init();
}

void loop() {
}

#endif
