#include "UtilityToolApps.h"

#include "WatchyUi.h"

#include "AppDisplay.h"

namespace WatchyUtilityTools {
namespace {
void draw(const uint16_t counts[3]) {
  beginAppDisplay("BUTTON TESTER");
  char value[8];
  snprintf(value, sizeof(value), "%u", counts[0] + counts[1] + counts[2]);
  AppVisual::drawMetric({12, 32, 176, 62}, "BUTTON EVENTS", value);
  snprintf(value, sizeof(value), "%u", counts[0]);
  AppVisual::drawDataRow(120, "MENU", value, true);
  snprintf(value, sizeof(value), "%u", counts[1]);
  AppVisual::drawDataRow(141, "UP", value);
  snprintf(value, sizeof(value), "%u", counts[2]);
  AppVisual::drawDataRow(162, "DOWN", value);
  WatchyUi::Widget::footer("PRESS BUTTONS  BACK EXIT");
  finishAppDisplay();
}
} // namespace
void runButtonTester() {
  WatchyUi::Input::begin();
  uint16_t counts[3] = {};
  draw(counts);
  while (true) {
    WatchyUi::Event event = WatchyUi::Input::wait();
    if (event == WatchyUi::Event::BACK) return;
    if (event == WatchyUi::Event::MENU || event == WatchyUi::Event::UP ||
        event == WatchyUi::Event::DOWN) {
      uint8_t index = event == WatchyUi::Event::UP ? 1
                      : event == WatchyUi::Event::DOWN ? 2 : 0;
      counts[index]++;
      draw(counts);
    }
  }
}
#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderButtonTesterPreview(uint8_t view) {
  const uint16_t counts[] = {static_cast<uint16_t>(view == 0 ? 0 : 3),
                             static_cast<uint16_t>(view == 0 ? 0 : 2),
                             static_cast<uint16_t>(view == 0 ? 0 : 4)};
  draw(counts);
}
#endif
} // namespace WatchyUtilityTools
