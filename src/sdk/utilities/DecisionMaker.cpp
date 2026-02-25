#include "UtilityToolApps.h"

#include "WatchyUi.h"

#include <esp_system.h>

#include "AppDisplay.h"
#include "UtilitySupport.h"

namespace WatchyUtilityTools {
namespace {
void generate(char *output, size_t) {
  snprintf(output, 40, "%s", esp_random() & 1 ? "YES" : "NO");
}
void draw(const char *value) {
  beginAppDisplay("DECISION MAKER");
  AppVisual::drawMetric({12, 42, 176, 104}, "ANSWER", value, -1.0f,
                        "A deterministic prompt result");
  WatchyUi::Widget::footer("SELECT NEW  BACK EXIT");
  finishAppDisplay();
}
} // namespace
void runDecisionMaker() { runGenerator(generate, draw); }
#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderDecisionMakerPreview(uint8_t view) { draw(view == 0 ? "YES" : "NO"); }
#endif
} // namespace WatchyUtilityTools
