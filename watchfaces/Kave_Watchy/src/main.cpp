#include <Arduino.h>
#include <KaveWatchy.h>
#include "../../../src/settings/settings.h"

KaveWatchy watchy(settings);

void setup() {
  watchy.init();
}

void loop() {
  // put your main code here, to run repeatedly:
}