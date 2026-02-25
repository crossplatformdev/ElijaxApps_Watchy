#include "UtilityToolApps.h"

#include "WatchyUi.h"

#include <esp_system.h>

#include "AppDisplay.h"
#include "UtilitySupport.h"

namespace WatchyUtilityTools {
namespace {
void generate(char *output, size_t) {
  snprintf(output, 40, "%lu", static_cast<unsigned long>(esp_random() % 6 + 1));
}
void draw(const char *result) {
  beginAppDisplay("D6 DICE");
  drawDieFace(6, static_cast<uint8_t>(atoi(result)));
  WatchyUi::Widget::footer("SELECT NEW  BACK EXIT");
  finishAppDisplay();
}
} // namespace
void runD6Dice() { runGenerator(generate, draw); }
#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderD6DicePreview(uint8_t) { draw("4"); }
#endif
} // namespace WatchyUtilityTools
