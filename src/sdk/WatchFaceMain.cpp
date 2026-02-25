#include "WatchyUi.h"
#include "settings.h"
#ifdef WATCHY_DETERMINISTIC_GALLERY
#include "DeterministicGallery.h"
#endif

void setup() {
#ifdef WATCHY_POWER_DIAGNOSTICS
  Serial.begin(115200);
  uint32_t serialDeadline = millis() + 2000;
  while (!Serial && static_cast<int32_t>(serialDeadline - millis()) > 0) {
    delay(10);
  }
  Serial.println("@WATCHY_POWER setup()");
#endif
#ifndef WATCHY_DETERMINISTIC_GALLERY
  WatchySdk::settings = settings;
  WatchySdk::init();
#endif
}

void loop() {
#ifdef WATCHY_DETERMINISTIC_GALLERY
  WatchyDemo::runDeterministicGallery();
#endif
}