#include "UtilityToolApps.h"

#include "WatchyUi.h"

#include <esp_system.h>

#include "AppDisplay.h"
#include "UtilitySupport.h"

namespace WatchyUtilityTools {
namespace {
void generate(char *output, size_t) {
  snprintf(output, 40, "%lu", static_cast<unsigned long>(esp_random() % 1000));
}
void draw(const char *value) {
  beginAppDisplay("RANDOM");
  AppVisual::drawMetric({12, 42, 176, 104}, "RANDOM RESULT", value, -1.0f,
                        "Range 0 to 999");
  WatchyUi::Widget::footer("SELECT NEW  BACK EXIT");
  finishAppDisplay();
}
} // namespace
void runRandomNumber() { runGenerator(generate, draw); }
#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderRandomNumberPreview(uint8_t) { draw("742"); }
#endif
} // namespace WatchyUtilityTools
