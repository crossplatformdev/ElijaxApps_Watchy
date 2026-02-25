#include "WatchyUi.h"
#include "settings.h"
#ifdef WATCHY_DETERMINISTIC_GALLERY
#include "DeterministicGallery.h"
#endif

void setup() {
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