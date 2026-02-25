#include <Arduino.h>
#include <SundialWatchy.h>

#include "../../../src/settings/settings.h"

SundialWatchy watchy(settings);

void setup() {
  watchy.init();
}

void loop() {
  // put your main code here, to run repeatedly:
}