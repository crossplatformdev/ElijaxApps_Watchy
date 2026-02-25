#include "WatchfaceSelector.h"
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
                          : WATCHFACE_7_SEG;
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

WatchfaceSelector::WatchfaceSelector(const watchySettings &settings)
    : Watchy(settings), sevenSegment(settings), dos(settings),
      macPaint(settings), mario(settings), pokemon(settings),
      starryHorizon(settings), tetris(settings) {}

void WatchfaceSelector::drawWatchFace() {
  display.setTextWrap(true);
  display.setTextSize(1);

  switch (getSelectedWatchface()) {
  case WATCHFACE_BASIC:
    Watchy::drawWatchFace();
    break;
  case WATCHFACE_DOS:
    dos.currentTime = currentTime;
    dos.drawWatchFace();
    break;
  case WATCHFACE_MACPAINT:
    macPaint.currentTime = currentTime;
    macPaint.drawWatchFace();
    break;
  case WATCHFACE_MARIO:
    mario.currentTime = currentTime;
    mario.drawWatchFace();
    break;
  case WATCHFACE_POKEMON:
    pokemon.currentTime = currentTime;
    pokemon.drawWatchFace();
    break;
  case WATCHFACE_STARRY_HORIZON:
    starryHorizon.currentTime = currentTime;
    starryHorizon.drawWatchFace();
    break;
  case WATCHFACE_TETRIS:
    tetris.currentTime = currentTime;
    tetris.drawWatchFace();
    break;
  case WATCHFACE_7_SEG:
  default:
    sevenSegment.currentTime = currentTime;
    sevenSegment.drawWatchFace();
    break;
  }
}

bool WatchfaceSelector::updateWatchFaceData() {
  if (getSelectedWatchface() != WATCHFACE_7_SEG) {
    return false;
  }
  return sevenSegment.updateWatchFaceData();
}

