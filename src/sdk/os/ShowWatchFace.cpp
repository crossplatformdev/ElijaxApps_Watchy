#include "WatchyUi.h"
#include "HeartRate.h"
#include "Watchy.h"

#include "WatchFaceRegistry.h"

namespace {

void showWatchFaceImpl(bool updateData, Watchy *watchy) {
  // Heart-rate monitoring is controlled centrally; do not gate per-face here.
  if (updateData) {
    if (watchy != nullptr) {
      watchy->updateWatchFaceData();
    } else {
      WatchySdk::updateWatchFaceData();
    }
  }
  WatchyUi::Screen::beginCanvas();
  if (watchy != nullptr) {
    watchy->drawWatchFace();
  } else {
    WatchySdk::drawWatchFace();
  }
  WatchyUi::Screen::present(WATCHFACE_STATE, false);
  #ifdef WATCHY_POWER_DIAGNOSTICS
  Serial.println("@WATCHY_POWER showWatchFace: after present");
  #endif
}

} // namespace

void Watchy::showWatchFace(bool updateData) {
  showWatchFaceImpl(updateData, this);
}

void WatchySdk::showWatchFace(bool updateData) {
  showWatchFaceImpl(updateData, nullptr);
}
