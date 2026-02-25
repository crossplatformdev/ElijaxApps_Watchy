#include <Arduino.h>
#include "SundialWatchy.h"

#include "../../../settings/settings.h"

static SundialWatchy watchy(settings);
#ifdef WATCHY_STANDALONE_WATCHFACE
void setup() {
  watchy.init();
}

void loop() {
  // put your main code here, to run repeatedly:
}
#endif // WATCHY_STANDALONE_WATCHFACE