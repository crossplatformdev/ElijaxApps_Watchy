#include "WatchyUi.h"

#include <stdint.h>

#include "TimeSupport.h"
#include "TimeToolApps.h"

namespace WatchyTimeTools {

void runStopwatch() {
  configureButtons();
  bool running = false;
  uint32_t accumulatedMs = 0;
  uint32_t startedAt = 0;
  uint32_t lapMs = 0;
  uint32_t displayedSecond = UINT32_MAX;
  bool refreshDue = true;

  while (true) {
    uint32_t elapsedMs = accumulatedMs + (running ? millis() - startedAt : 0);
    uint32_t elapsedSecond = elapsedMs / 1000;
    if (refreshDue || elapsedSecond != displayedSecond) {
      char lapText[24];
      snprintf(lapText, sizeof(lapText), "LAP %02lu:%02lu",
               static_cast<unsigned long>(lapMs / 60000),
               static_cast<unsigned long>(lapMs / 1000 % 60));
      drawTimer("STOPWATCH", elapsedSecond, running ? "RUNNING" : "PAUSED",
                "SELECT START/STOP UP LAP DOWN RESET", lapText, !refreshDue,
                static_cast<float>(elapsedSecond % 60) / 60.0f);
      displayedSecond = elapsedSecond;
      refreshDue = false;
    }

    elapsedMs = accumulatedMs + (running ? millis() - startedAt : 0);
    uint32_t waitMs = running ? 1000 - elapsedMs % 1000 : UINT32_MAX;
    WatchyUi::Event event = WatchyUi::Input::wait(waitMs);
    uint32_t eventElapsedMs =
        accumulatedMs + (running ? millis() - startedAt : 0);
    if (event == WatchyUi::Event::BACK) {
      return;
    }
    if (event == WatchyUi::Event::MENU) {
      if (running) {
        accumulatedMs = eventElapsedMs;
      } else {
        startedAt = millis();
      }
      running = !running;
      refreshDue = true;
    } else if (event == WatchyUi::Event::UP) {
      lapMs = eventElapsedMs;
      refreshDue = true;
    } else if (event == WatchyUi::Event::DOWN) {
      if (!running) {
        accumulatedMs = 0;
        lapMs = 0;
      }
      refreshDue = true;
    }
  }
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderStopwatchPreview(uint8_t view) {
  drawTimer("STOPWATCH", 12 * 60 + 34,
            view == 0 ? "PAUSED" : "RUNNING",
            "SELECT START/STOP UP LAP DOWN RESET",
            view == 2 ? "LAP 04:21" : "LAP --:--", false,
            34.0f / 60.0f);
}
#endif

} // namespace WatchyTimeTools
