#include <Arduino.h>
#include "KaveWatchy.h"
#include "../../../settings/settings.h"

static KaveWatchy watchy(settings);
#ifdef WATCHY_STANDALONE_WATCHFACE
void setup() {
  watchy.init();
}

void loop() {
  // put your main code here, to run repeatedly:
}
#endif // WATCHY_STANDALONE_WATCHFACE