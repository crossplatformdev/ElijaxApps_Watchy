#include "./watchy/Watchy.h"
#include "./settings/settings.h"

Watchy watchy(settings);

void setup() {
  watchy.init(DATETIME_NOW);
}

void loop() {
  // Watchy uses deep sleep; nothing required here.
}
