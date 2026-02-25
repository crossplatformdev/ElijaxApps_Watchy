#include "UtilityToolApps.h"

#include "WatchyUi.h"

#include <esp_system.h>

#include "AppDisplay.h"
#include "UtilitySupport.h"

namespace WatchyUtilityTools {
namespace {
void generate(char *output, size_t) {
  snprintf(output, 40, "%s", esp_random() & 1 ? "HEADS" : "TAILS");
}
void draw(const char *result) {
  beginAppDisplay("COIN FLIP");
  drawCoinFace(result);
  WatchyUi::Widget::footer("SELECT NEW  BACK EXIT");
  finishAppDisplay();
}
} // namespace
void runCoinFlip() { runGenerator(generate, draw); }
#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderCoinFlipPreview(uint8_t view) { draw(view == 0 ? "HEADS" : "TAILS"); }
#endif
} // namespace WatchyUtilityTools
