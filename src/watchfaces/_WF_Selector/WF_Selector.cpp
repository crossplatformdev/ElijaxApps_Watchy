#include "WatchfaceSelector.h"
#include "settings.h"
#ifdef WATCHY_DETERMINISTIC_GALLERY
#include "demo/DeterministicGallery.h"
#endif

WatchfaceSelector watchy(settings);

void setup() {
#ifndef WATCHY_DETERMINISTIC_GALLERY
  watchy.init();
#endif
}

void loop() {
#ifdef WATCHY_DETERMINISTIC_GALLERY
  WatchyDemo::runDeterministicGallery(watchy);
#endif
}