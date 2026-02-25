#include "UtilityToolApps.h"

#include "WatchyUi.h"

#include <esp_sleep.h>

#include "AppDisplay.h"

namespace WatchyUtilityTools {
namespace {
const char *name(esp_sleep_wakeup_cause_t cause) {
  const char *label = "COLD RESET / OTHER";
  switch (cause) {
  case ESP_SLEEP_WAKEUP_EXT0: label = "EXT0 / USB or RTC"; break;
  case ESP_SLEEP_WAKEUP_EXT1: label = "EXT1 / BUTTON"; break;
  case ESP_SLEEP_WAKEUP_TIMER: label = "TIMER / MINUTE"; break;
  case ESP_SLEEP_WAKEUP_TOUCHPAD: label = "TOUCHPAD"; break;
  case ESP_SLEEP_WAKEUP_ULP: label = "ULP"; break;
  default: break;
  }
  return label;
}
void draw(const char *reason, int raw) {
  beginAppDisplay("WAKE REASON");
  AppVisual::drawStatusIcon({79, 36, 42, 42}, AppVisual::StatusIcon::TIME, true);
  WatchyUi::Canvas::centeredText({12, 88, 176, 28}, reason, 2,
                                 WatchyUi::Theme::foreground());
  char rawText[12];
  snprintf(rawText, sizeof(rawText), "%d", raw);
  AppVisual::drawDataRow(146, "RAW CODE", rawText, true);
  AppVisual::drawDataRow(174, "TYPE", "Wake source");
  finishAppDisplay();
}
} // namespace
void runWakeReason() {
  esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
  draw(name(cause), static_cast<int>(cause));
}
#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderWakeReasonPreview(uint8_t view) {
  draw(view == 0 ? "TIMER / MINUTE" : "BUTTON", view == 0 ? 4 : 2);
}
#endif
} // namespace WatchyUtilityTools
