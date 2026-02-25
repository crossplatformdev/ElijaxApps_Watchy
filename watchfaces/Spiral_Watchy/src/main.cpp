#include <Arduino.h>
#include <SpiralWatchy.h>

#include "../../../src/settings/settings.h"

SpiralWatchy watchy(settings);

void setup() {
  watchy.init();
}

void loop() {
  // put your main code here, to run repeatedly:
}