#include <Arduino.h>
#include "BetonWatchy.h"
#include "../../../settings/settings.h"

static BetonWatchy watchy(settings);
#ifdef WATCHY_STANDALONE_WATCHFACE
void setup() {
  watchy.init();
}

void loop() {
  // put your main code here, to run repeatedly:
}
#endif // WATCHY_STANDALONE_WATCHFACE