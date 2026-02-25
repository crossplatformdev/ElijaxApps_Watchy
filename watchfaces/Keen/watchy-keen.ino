#include "../../src/watchy/Watchy.h"
#include "keen.h"
#include "../../src/settings/settings.h"

WatchyKeen watchy(settings);

void setup() {
  watchy.init();
}

void loop() {}
