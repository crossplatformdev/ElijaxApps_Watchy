#include "WatchyUi.h"
#include "HeartRate.h"
#include "Watchy.h"

#include "WatchFaceRegistry.h"

namespace {

void showWatchFaceImpl(bool updateData, Watchy *watchy) {
  setWatchfaceHeartRateMonitoring(
      getSelectedWatchface() == WATCHFACE_7_SEG);
  if (updateData) {
    if (watchy != nullptr) {
      watchy->updateWatchFaceData();
    } else {
      WatchySdk::updateWatchFaceData();
    }
  }
  if (watchy != nullptr) {
    watchy->ensureDisplayInitialized();
  } else {
    WatchySdk::ensureDisplayInitialized();
  }
  Watchy::display.epd2.asyncPowerOn();

  WatchyUi::Screen::beginCanvas();
  if (watchy != nullptr) {
    watchy->drawWatchFace();
  } else {
    WatchySdk::drawWatchFace();
  }
  // Only the very first boot paint uses a full flashing refresh; every
  // later watch face update stays partial to avoid corrupting the panel.
  WatchyUi::Screen::present(WATCHFACE_STATE, false);
}

} // namespace

void Watchy::showWatchFace(bool updateData) {
  showWatchFaceImpl(updateData, this);
}

void WatchySdk::showWatchFace(bool updateData) {
  showWatchFaceImpl(updateData, nullptr);
}
