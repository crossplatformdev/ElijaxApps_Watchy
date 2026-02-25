#include <Arduino.h>
#include <BetonWatchy.h>
#include "../../../src/settings/settings.h"

BetonWatchy watchy(settings);

void setup() {
  watchy.init();
}

void loop() {
  // put your main code here, to run repeatedly:
}