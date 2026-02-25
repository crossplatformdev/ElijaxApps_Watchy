#include "WatchyUi.h"

#include <stdint.h>

#include "TimeSupport.h"
#include "TimeToolApps.h"

namespace WatchyTimeTools {
namespace {

void drawInterval(bool workPhase, uint16_t cycle, uint32_t remaining,
                  uint32_t phaseSeconds, bool running,
                  bool valueOnly = false) {
  char state[32];
  snprintf(state, sizeof(state), "%s  ROUND %u",
           workPhase ? "WORK" : "REST", cycle);
  drawTimer("INTERVALS", remaining, state,
            running ? "SELECT PAUSE"
                    : "SELECT START UP WORK DOWN REST", nullptr, valueOnly,
            phaseSeconds == 0 ? 1.0f
                              : 1.0f - static_cast<float>(remaining) /
                                           phaseSeconds);
}

} // namespace

void runIntervals() {
  configureButtons();
  uint16_t workSeconds = 30;
  uint16_t restSeconds = 10;
  uint16_t cycle = 1;
  bool workPhase = true;
  bool running = false;
  uint32_t remaining = workSeconds;
  uint32_t deadline = 0;
  uint32_t displayedRemaining = UINT32_MAX;
  bool refreshDue = true;

  while (true) {
    if (running) {
      int32_t remainingMs = static_cast<int32_t>(deadline - millis());
      if (remainingMs <= 0) {
        pulseMotor(workPhase ? 250 : 500);
        if (!workPhase) {
          cycle++;
        }
        workPhase = !workPhase;
        remaining = workPhase ? workSeconds : restSeconds;
        deadline = millis() + remaining * 1000UL;
      } else {
        remaining = (remainingMs + 999) / 1000;
      }
    }
    if (refreshDue || remaining != displayedRemaining) {
      drawInterval(workPhase, cycle, remaining,
                   workPhase ? workSeconds : restSeconds, running,
                   !refreshDue);
      displayedRemaining = remaining;
      refreshDue = false;
    }
    uint32_t waitMs = UINT32_MAX;
    if (running) {
      int32_t remainingMs = static_cast<int32_t>(deadline - millis());
      if (remainingMs <= 0) {
        waitMs = 0;
      } else {
        waitMs = static_cast<uint32_t>(remainingMs) % 1000;
        if (waitMs == 0) waitMs = 1000;
      }
    }
    WatchyUi::Event event = WatchyUi::Input::wait(waitMs);
    if (event == WatchyUi::Event::BACK) {
      return;
    }
    if (event == WatchyUi::Event::MENU) {
      if (running) {
        int32_t remainingMs = static_cast<int32_t>(deadline - millis());
        remaining = remainingMs > 0 ? (remainingMs + 999) / 1000 : 0;
        running = false;
      } else {
        deadline = millis() + remaining * 1000UL;
        running = true;
      }
      refreshDue = true;
    } else if (!running && event == WatchyUi::Event::UP) {
      workSeconds = min<uint16_t>(300, workSeconds + 5);
      if (workPhase) {
        remaining = workSeconds;
      }
      refreshDue = true;
    } else if (!running && event == WatchyUi::Event::DOWN) {
      restSeconds = min<uint16_t>(120, restSeconds + 5);
      if (!workPhase) {
        remaining = restSeconds;
      }
      refreshDue = true;
    }
  }
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderIntervalTimerPreview(uint8_t view) {
  drawInterval(view != 2, view == 0 ? 1 : 3,
               view == 0 ? 30 : view == 1 ? 23 : 7,
               view == 2 ? 10 : 30, view != 0);
}
#endif

} // namespace WatchyTimeTools
