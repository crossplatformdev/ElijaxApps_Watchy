#include <Watchy.h>
#include "app/HeartRate.h"
#include "sdk/WatchyUi.h"
#include "watchfaces/_WF_Selector/WatchfaceRegistry.h"

void Watchy::showWatchFace(bool updateData) {
  setWatchfaceHeartRateMonitoring(getSelectedWatchface() == WATCHFACE_7_SEG);
  if (updateData) {
    updateWatchFaceData();
  }
  display.epd2.asyncPowerOn();

  WatchyUi::Screen::beginCanvas();
  drawWatchFace();
  WatchyUi::Screen::present(WATCHFACE_STATE);
}