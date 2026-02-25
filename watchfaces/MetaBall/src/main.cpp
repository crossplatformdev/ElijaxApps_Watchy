#include <Arduino.h>
#include <MetaBallWatchy.h>

#include "../../../src/settings/settings.h"

MetaBallWatchy watchy(settings);

void setup() {
  watchy.init();
}

void loop() {
  // put your main code here, to run repeatedly:
}