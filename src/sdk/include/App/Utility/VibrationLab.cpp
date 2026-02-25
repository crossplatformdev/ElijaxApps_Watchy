#include "UtilityToolApps.h"

#include "WatchyUi.h"
#include "Watchy.h"

#include "AppDisplay.h"

namespace WatchyUtilityTools {
namespace {
const char *const names[] = {"TAP", "DOUBLE", "TRIPLE", "LONG", "HEARTBEAT"};
const uint8_t pulses[] = {1, 2, 3, 1, 2};
void draw(uint8_t pattern) {
  beginAppDisplay("VIBRATION LAB");
  AppVisual::drawStatusIcon({79, 37, 42, 42}, AppVisual::StatusIcon::TIME, true);
  WatchyUi::Canvas::centeredText({0, 87, 200, 24}, names[pattern], 3,
                                 WatchyUi::Theme::foreground());
  AppVisual::drawSignalBars({44, 120, 112, 42}, pulses[pattern], 3, true);
  WatchyUi::Widget::footer("UP/DOWN PATTERN  SELECT PLAY  BACK");
  finishAppDisplay();
}
void play(uint8_t pattern) {
  for (uint8_t pulse = 0; pulse < pulses[pattern]; pulse++) {
    Watchy::vibMotor(pattern == 3 ? 800 : pattern == 4 && pulse == 1 ? 180 : 80,
             1);
    if (pulse + 1 < pulses[pattern]) {
      WatchyUi::deepSleepDelay(pattern == 4 ? 130 : 100);
    }
  }
}
} // namespace
void runVibrationLab() {
  WatchyUi::Input::begin();
  uint8_t pattern = 0;
  draw(pattern);
  while (true) {
    WatchyUi::Event event = WatchyUi::Input::wait();
    if (event == WatchyUi::Event::BACK) return;
    if (event == WatchyUi::Event::UP) {
      pattern = WatchyUi::ListView::previous(pattern, 5);
      draw(pattern);
    } else if (event == WatchyUi::Event::DOWN) {
      pattern = WatchyUi::ListView::next(pattern, 5);
      draw(pattern);
    } else if (event == WatchyUi::Event::MENU) {
      play(pattern);
    }
  }
}
#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderVibrationLabPreview(uint8_t view) { draw(view == 0 ? 0 : 4); }
#endif
} // namespace WatchyUtilityTools
