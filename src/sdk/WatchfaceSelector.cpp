#include "WatchyUi.h"
#include "Watchy.h"
#include "WatchFaceRegistry.h"

#include "Watchy_7_SEG.h"
#include "DSEG7_Classic_Bold_25.h"
#include "DSEG7_Classic_Regular_15.h"
#include "DSEG7_Classic_Regular_39.h"
#include "DSEG7_Classic_Bold_53.h"
#include "Seven_Segment10pt7b.h"
#include "Px437_IBM_BIOS5pt7b.h"

#include <Preferences.h>

RTC_DATA_ATTR uint8_t selectedWatchface = WATCHFACE_7_SEG;
RTC_DATA_ATTR bool selectedWatchfaceLoaded = false;

const char *const watchfaceNames[WATCHFACE_COUNT] = {
    "7 SEG", "Basic", "DOS", "MacPaint", "Mario", "Pokemon",
    "Starry Horizon", "Tetris"};

static_assert(sizeof(watchfaceNames) / sizeof(watchfaceNames[0]) ==
                  WATCHFACE_COUNT,
              "Watchface registry is incomplete");

uint8_t getSelectedWatchface() {
  if (!selectedWatchfaceLoaded) {
    Preferences preferences;
    if (preferences.begin("watchy-ui", true)) {
      selectedWatchface = preferences.getUChar("face", WATCHFACE_7_SEG);
      preferences.end();
    }
    selectedWatchfaceLoaded = true;
  }
  if (selectedWatchface >= WATCHFACE_COUNT) {
    selectedWatchface = WATCHFACE_7_SEG;
  }
  return selectedWatchface;
}

void setSelectedWatchface(uint8_t watchfaceId) {
  saveSelectedWatchface(watchfaceId);
}

bool saveSelectedWatchface(uint8_t watchfaceId) {
  uint8_t candidate = watchfaceId < WATCHFACE_COUNT
                          ? watchfaceId
                          : static_cast<uint8_t>(WATCHFACE_7_SEG);
  Preferences preferences;
  if (!preferences.begin("watchy-ui", false)) {
    return false;
  }
  bool saved = preferences.putUChar("face", candidate) == sizeof(candidate);
  preferences.end();
  if (saved) {
    selectedWatchface = candidate;
    selectedWatchfaceLoaded = true;
  }
  return saved;
}

namespace {

void drawBasicFace(Watchy &watch) {
  Watchy::display.setFont(&DSEG7_Classic_Bold_53);
  Watchy::display.setCursor(5, 53 + 60);
  if (watch.currentTime.Hour < 10) {
    Watchy::display.print("0");
  }
  Watchy::display.print(watch.currentTime.Hour);
  Watchy::display.print(":");
  if (watch.currentTime.Minute < 10) {
    Watchy::display.print("0");
  }
  Watchy::display.println(watch.currentTime.Minute);
}

bool noHeartRateRefresh(Watchy &) { return false; }
bool noDataUpdate(Watchy &) { return false; }

struct WatchfaceOps {
  void (*draw)(Watchy &watch);
  bool (*updateData)(Watchy &watch);
  bool (*refreshHeartRate)(Watchy &watch);
};

const WatchfaceOps watchfaceTable[WATCHFACE_COUNT] = {
    {Watchy7SEG::drawWatchFace, Watchy7SEG::updateWatchFaceData,
     [](Watchy &watch) -> bool {
       Watchy7SEG::refreshHeartRate(watch);
       return true;
     }},
    {drawBasicFace, noDataUpdate, noHeartRateRefresh},
    {WatchyDOS::drawWatchFace, noDataUpdate, noHeartRateRefresh},
    {WatchyMacPaint::drawWatchFace, noDataUpdate, noHeartRateRefresh},
    {WatchyMario::drawWatchFace, noDataUpdate, noHeartRateRefresh},
    {WatchyPokemon::drawWatchFace, noDataUpdate, noHeartRateRefresh},
    {WatchyStarryHorizon::drawWatchFace, noDataUpdate, noHeartRateRefresh},
    {WatchyTetris::drawWatchFace, noDataUpdate, noHeartRateRefresh},
};

const WatchfaceOps &activeWatchfaceOps() {
  return watchfaceTable[getSelectedWatchface()];
}

} // namespace

namespace WatchySdk {

void drawWatchFace() {
  Watchy watch(settings);
  watch.currentTime = currentTime;
  activeWatchfaceOps().draw(watch);
}

bool updateWatchFaceData() {
  Watchy watch(settings);
  watch.currentTime = currentTime;
  return activeWatchfaceOps().updateData(watch);
}

bool refreshWatchFaceHeartRate() {
  Watchy watch(settings);
  watch.currentTime = currentTime;
  return activeWatchfaceOps().refreshHeartRate(watch);
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
void drawGalleryWatchface(uint8_t watchfaceId, const tmElements_t &fixedTime) {
  selectedWatchface = watchfaceId < WATCHFACE_COUNT
                          ? watchfaceId
                          : static_cast<uint8_t>(WATCHFACE_7_SEG);
  selectedWatchfaceLoaded = true;
  currentTime = fixedTime;
  Watchy watch(settings);
  watch.currentTime = fixedTime;
  activeWatchfaceOps().draw(watch);
}
#endif

} // namespace WatchySdk

void Watchy::drawWatchFace() { activeWatchfaceOps().draw(*this); }

bool Watchy::updateWatchFaceData() {
  return activeWatchfaceOps().updateData(*this);
}

bool Watchy::refreshWatchFaceHeartRate() {
  return activeWatchfaceOps().refreshHeartRate(*this);
}
