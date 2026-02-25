#include <Arduino.h>
#include <ShadowClockWatchy.h>

#include "../../../src/settings/settings.h"

ShadowClockWatchy watchy(settings);

void setup() {
  watchy.init();
}

void loop() {
  // put your main code here, to run repeatedly:
}