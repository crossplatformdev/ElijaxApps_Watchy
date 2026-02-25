#include "WatchyUi.h"
#include "Watchy.h"
#include "WatchFaceRegistry.h"

#include "HeartRate.h"

#include "Watchy_7_SEG.h"
#include "DSEG7_Classic_Bold_25.h"
#include "DSEG7_Classic_Regular_15.h"
#include "DSEG7_Classic_Regular_39.h"
#include "DSEG7_Classic_Bold_53.h"
#include "Seven_Segment10pt7b.h"
#include "Px437_IBM_BIOS5pt7b.h"
#include "icons.h"

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

bool noDataUpdate(Watchy &) { return false; }

void drawGenericHeartRate() {
  using namespace WatchyUi;
  uint8_t face = getSelectedWatchface();
  Bounds hrBounds{static_cast<int16_t>(DISPLAY_WIDTH - 48), static_cast<int16_t>(DISPLAY_HEIGHT - 24), 48, 24};
  const GFXfont *font = &FreeMonoBold9pt7b;
  // Choose innocuous locations and fonts per face when possible.
  switch (face) {
    case WATCHFACE_7_SEG:
      hrBounds = {static_cast<int16_t>(DISPLAY_WIDTH - 48), static_cast<int16_t>(DISPLAY_HEIGHT - 24), 48, 24};
      font = &Seven_Segment10pt7b;
      return;
    case WATCHFACE_BASIC: // Basic
      hrBounds = {static_cast<int16_t>(DISPLAY_WIDTH - 60), 4, 56, 24};
      font = &Seven_Segment10pt7b;
      break;
    case WATCHFACE_DOS: // DOS
      hrBounds = {static_cast<int16_t>(DISPLAY_WIDTH - 80), 4, 76, 24};
      font = &Px437_IBM_BIOS5pt7b;
      break;
    case WATCHFACE_MACPAINT: // MacPaint
      hrBounds = {static_cast<int16_t>(DISPLAY_WIDTH - 60), 4, 56, 24};
      font = &FreeMonoBold9pt7b;
      break;
    case WATCHFACE_MARIO: // Mario
      hrBounds = {static_cast<int16_t>(DISPLAY_WIDTH - 56), 4, 52, 24};
      font = &FreeMonoBold9pt7b;
      break;
    case WATCHFACE_POKEMON: // Pokemon
      hrBounds = {static_cast<int16_t>(DISPLAY_WIDTH - 56), static_cast<int16_t>(DISPLAY_HEIGHT - 28), 52, 24};
      font = &FreeMonoBold9pt7b;
      break;
    case WATCHFACE_STARRY_HORIZON: // Starry Horizon
      hrBounds = {static_cast<int16_t>(DISPLAY_WIDTH - 56), 4, 52, 24};
      font = &FreeMonoBold9pt7b;
      break;
    case WATCHFACE_TETRIS: // Tetris
      hrBounds = {static_cast<int16_t>(DISPLAY_WIDTH - 56), static_cast<int16_t>(DISPLAY_HEIGHT - 28), 52, 24};
      font = &Seven_Segment10pt7b;
      break;
    default:
      break;
  }

  // Draw only the heart-rate region into the canvas, then present that window.
  Watchy::display.fillRect(hrBounds.x, hrBounds.y, hrBounds.width, hrBounds.height,
                   Theme::background());
  int16_t centerX = hrBounds.x + HEART_ICON_WIDTH / 2;
  int16_t centerY = hrBounds.y + hrBounds.height / 2;
  Watchy::display.drawBitmap(centerX - HEART_ICON_WIDTH / 2,
                     centerY - HEART_ICON_HEIGHT / 2,
                     heart, HEART_ICON_WIDTH, HEART_ICON_HEIGHT,
                     Theme::foreground());
  Watchy::display.setFont(font);
  Watchy::display.setTextSize(1);
  Watchy::display.setTextColor(Theme::foreground());
  char bpmText[6];
  snprintf(bpmText, sizeof(bpmText), "%u", watchfaceHeartRateBpm());
  Bounds textBounds{static_cast<int16_t>(hrBounds.x + HEART_ICON_WIDTH + 2),
                    hrBounds.y,
                    static_cast<int16_t>(hrBounds.width - HEART_ICON_WIDTH - 2),
                    hrBounds.height};
  int16_t textX, textY;
  uint16_t textWidth, textHeight;
  Watchy::display.getTextBounds(bpmText, 0, 0, &textX, &textY,
                                &textWidth, &textHeight);
  if (textWidth > textBounds.width || textHeight > textBounds.height) {
    Watchy::display.setFont();
  }
  Canvas::centeredText(textBounds, bpmText, Theme::foreground());
  Screen::invalidate(hrBounds);
}

bool genericRefreshHeartRate(Watchy &) {
  drawGenericHeartRate();
  WatchyUi::Screen::presentDirty(WATCHFACE_STATE);
  return true;
}

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
    {drawBasicFace, noDataUpdate, genericRefreshHeartRate},
    {WatchyDOS::drawWatchFace, noDataUpdate, genericRefreshHeartRate},
    {WatchyMacPaint::drawWatchFace, noDataUpdate, genericRefreshHeartRate},
    {WatchyMario::drawWatchFace, noDataUpdate, genericRefreshHeartRate},
    {WatchyPokemon::drawWatchFace, noDataUpdate, genericRefreshHeartRate},
    {WatchyStarryHorizon::drawWatchFace, noDataUpdate, genericRefreshHeartRate},
    {WatchyTetris::drawWatchFace, noDataUpdate, genericRefreshHeartRate},
};

const WatchfaceOps &activeWatchfaceOps() {
  return watchfaceTable[getSelectedWatchface()];
}

void drawActiveWatchface(Watchy &watch) {
  activeWatchfaceOps().draw(watch);
  if (getSelectedWatchface() != WATCHFACE_7_SEG) {
    drawGenericHeartRate();
  }
}

} // namespace

namespace WatchySdk {

void drawWatchFace() {
  WatchyUi::Screen::beginCanvas();
  Watchy watch(settings);
  watch.currentTime = currentTime;
  drawActiveWatchface(watch);
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
  WatchyUi::Screen::beginCanvas();
  selectedWatchface = watchfaceId < WATCHFACE_COUNT
                          ? watchfaceId
                          : static_cast<uint8_t>(WATCHFACE_7_SEG);
  selectedWatchfaceLoaded = true;
  currentTime = fixedTime;
  Watchy watch(settings);
  watch.currentTime = fixedTime;
  drawActiveWatchface(watch);
}
#endif

} // namespace WatchySdk

void Watchy::drawWatchFace() {
  WatchyUi::Screen::beginCanvas();
  drawActiveWatchface(*this);
}

bool Watchy::updateWatchFaceData() {
  return activeWatchfaceOps().updateData(*this);
}

bool Watchy::refreshWatchFaceHeartRate() {
  return activeWatchfaceOps().refreshHeartRate(*this);
}
