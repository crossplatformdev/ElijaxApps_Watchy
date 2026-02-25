#include "../watchy/Watchy.h"
#include "../sdk/UiSDK.h"
#include "../sdk/WatchfaceRegistry.h"

void Watchy::showWatchFace(bool partialRefresh) {
  display.epd2.asyncPowerOn();

  drawWatchFace();

  guiState = WATCHFACE_STATE;
  
  pinMode(MENU_BTN_PIN, INPUT);
  pinMode(BACK_BTN_PIN, INPUT);
  pinMode(UP_BTN_PIN, INPUT);
  pinMode(DOWN_BTN_PIN, INPUT);

  display.display(partialRefresh);
  display.epd2.powerOff();
}

void Watchy::drawWatchFace() {
  if (WatchfaceRegistry::kWatchfaceCount == 0) {
    return;
  }

  if (currentWatchfaceId >= WatchfaceRegistry::kWatchfaceCount) {
    currentWatchfaceId = 0;
  }

  uint8_t drawId = currentWatchfaceId;
  const WatchfaceRegistry::Entry *wf = &WatchfaceRegistry::kWatchfaces[drawId];
  if (wf->draw == nullptr) {
    // Hard guard: never call through a null function pointer.
    // Fall back to the first registered watchface.
    drawId = 0;
    currentWatchfaceId = 0;
    wf = &WatchfaceRegistry::kWatchfaces[drawId];
    if (wf->draw == nullptr) {
      return;
    }
  }

  // Theme inversion for third-party watchfaces.
  // Many faces hardcode GxEPD_BLACK / GxEPD_WHITE. We treat each face as being
  // authored for a default polarity, and invert at the display layer when the
  // live theme polarity differs.
  const WatchfacePolarity authored = UiSDK::polarityForFaceId(drawId);
  UiSDK::setPolarity(authored);

  const bool usesThemeColors = UiSDK::watchfaceUsesThemeColors(drawId);
  const bool invert = (authored != BASE_POLARITY) && !usesThemeColors;

  display.setInverted(invert);

  // Many third-party watchfaces do not clear the background, and some draw
  // assets using GxEPD_WHITE expecting a black background. Clear to the
  // authored background *after* setting inversion so the final output matches
  // BASE_POLARITY.
  if (!usesThemeColors) {
    display.fillScreen(UiSDK::getWatchfaceBg(authored));
  }

  wf->draw(*this);

  // Always restore normal rendering state.
  display.setInverted(false);
  UiSDK::setPolarity(BASE_POLARITY);
}
