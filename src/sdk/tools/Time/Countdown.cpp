#include "TimeSupport.h"
#include "TimeToolApps.h"

#include "WatchyUi.h"

namespace WatchyTimeTools {

void runCountdown() {
  runCountdownTimer(5 * 60UL, "COUNTDOWN", false);
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderCountdownPreview(uint8_t view) {
  drawTimer("COUNTDOWN", view == 0 ? 5 * 60 : view == 1 ? 4 * 60 + 12 : 0,
            view == 0 ? "READY" : view == 1 ? "RUNNING" : "FINISHED",
            "UP +1M DOWN -1M SELECT START", nullptr, false,
            view == 0 ? 0.0f : view == 1 ? 48.0f / 300.0f : 1.0f);
}
#endif

} // namespace WatchyTimeTools
