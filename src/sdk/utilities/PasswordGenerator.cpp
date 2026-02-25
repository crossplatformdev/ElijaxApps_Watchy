#include "UtilityToolApps.h"

#include "WatchyUi.h"

#include <esp_system.h>

#include "AppDisplay.h"
#include "UtilitySupport.h"

namespace WatchyUtilityTools {
namespace {
void generate(char *output, size_t) {
  constexpr char characters[] =
      "ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz23456789!@#$%";
  for (size_t index = 0; index < 12; index++) {
    output[index] = characters[esp_random() % (sizeof(characters) - 1)];
  }
  output[12] = '\0';
}
void draw(const char *value) {
  beginAppDisplay("PASSWORD GEN");
  AppVisual::drawStatusIcon({79, 39, 42, 42}, AppVisual::StatusIcon::INFO, true);
  AppVisual::drawDataRow(110, "PASSWORD", value, true);
  AppVisual::drawDataRow(140, "LENGTH", "12 characters");
  WatchyUi::Widget::footer("SELECT NEW  BACK EXIT");
  finishAppDisplay();
}
} // namespace
void runPasswordGenerator() { runGenerator(generate, draw); }
#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderPasswordGeneratorPreview(uint8_t) { draw("N7vK4!pQ2xLm"); }
#endif
} // namespace WatchyUtilityTools
