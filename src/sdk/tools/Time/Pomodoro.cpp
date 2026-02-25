#include "TimeSupport.h"
#include "TimeToolApps.h"

#include "WatchyUi.h"

namespace WatchyTimeTools {

void runPomodoro() {
  runCountdownTimer(25 * 60UL, "POMODORO", true);
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderPomodoroPreview(uint8_t view) {
  drawTimer("POMODORO",
            view == 0 ? 25 * 60 : view == 1 ? 18 * 60 + 42
                                       : view == 2 ? 4 * 60 + 12 : 0,
            view == 0 ? "READY" : view == 1 ? "FOCUS"
                                 : view == 2 ? "BREAK" : "FINISHED",
            "UP 25M DOWN 5M SELECT START", nullptr, false,
            view == 0 ? 0.0f : view == 1 ? 1.0f - 1122.0f / 1500.0f
                              : view == 2 ? 1.0f - 252.0f / 300.0f : 1.0f);
}
#endif

} // namespace WatchyTimeTools
