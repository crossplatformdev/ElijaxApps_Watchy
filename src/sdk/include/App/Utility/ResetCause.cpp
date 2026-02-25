#include "UtilityToolApps.h"

#include "WatchyUi.h"

#include <esp_system.h>

#include "AppDisplay.h"

namespace WatchyUtilityTools {
namespace {
const char *name(esp_reset_reason_t reason) {
  const char *label = "OTHER";
  switch (reason) {
  case ESP_RST_POWERON: label = "POWER ON"; break;
  case ESP_RST_EXT: label = "EXTERNAL PIN"; break;
  case ESP_RST_SW: label = "SOFTWARE"; break;
  case ESP_RST_PANIC: label = "PANIC"; break;
  case ESP_RST_INT_WDT: label = "INT WATCHDOG"; break;
  case ESP_RST_TASK_WDT: label = "TASK WATCHDOG"; break;
  case ESP_RST_WDT: label = "WATCHDOG"; break;
  case ESP_RST_DEEPSLEEP: label = "DEEP SLEEP"; break;
  case ESP_RST_BROWNOUT: label = "BROWNOUT"; break;
  default: break;
  }
  return label;
}
void draw(const char *reason, int raw) {
  beginAppDisplay("RESET REASON");
  AppVisual::drawStatusIcon({79, 36, 42, 42}, AppVisual::StatusIcon::INFO, true);
  WatchyUi::Canvas::centeredText({12, 88, 176, 28}, reason, 2,
                                 WatchyUi::Theme::foreground());
  char rawText[12];
  snprintf(rawText, sizeof(rawText), "%d", raw);
  AppVisual::drawDataRow(146, "RAW CODE", rawText, true);
  AppVisual::drawDataRow(174, "TYPE", "Reset source");
  finishAppDisplay();
}
} // namespace
void runResetCause() {
  esp_reset_reason_t reason = esp_reset_reason();
  draw(name(reason), static_cast<int>(reason));
}
#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderResetCausePreview(uint8_t view) {
  draw(view == 0 ? "POWER ON" : "PANIC", view == 0 ? 1 : 4);
}
#endif
} // namespace WatchyUtilityTools
