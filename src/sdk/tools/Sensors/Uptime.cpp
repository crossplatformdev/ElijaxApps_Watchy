#include "SensorToolApps.h"

#include "WatchyUi.h"

#include "AppDisplay.h"

namespace WatchySensorTools {
namespace {

constexpr WatchyUi::Bounds metricBounds{12, 42, 176, 88};

void drawUptime(uint32_t seconds) {
  uint32_t days = seconds / SECS_PER_DAY;
  uint8_t hours = seconds / SECS_PER_HOUR % 24;
  uint8_t minutes = seconds / SECS_PER_MIN % 60;
  uint8_t remainingSeconds = seconds % SECS_PER_MIN;
  char duration[24];
  snprintf(duration, sizeof(duration), "%lud %02uh %02um %02us",
           static_cast<unsigned long>(days), hours, minutes, remainingSeconds);
  AppVisual::drawMetric(metricBounds, "UPTIME", duration, -1.0f,
                        "Since cold boot");
}

void drawScreen(Watchy *watchy) {
  beginAppDisplay("UPTIME");
  drawUptime(watchy != nullptr ? watchy->uptimeSeconds()
                               : WatchySdk::uptimeSeconds());
  WatchyUi::Widget::footer("LIVE VALUE  BACK EXIT");
  finishAppDisplay();
}

} // namespace

void runUptime(Watchy *watchy) {
  WatchyUi::Input::begin();
  drawScreen(watchy);
  while (true) {
    if (WatchyUi::Input::wait(WatchyUi::Screen::liveViewRefreshIntervalMs) ==
        WatchyUi::Event::BACK) {
      return;
    }
    drawUptime(watchy != nullptr ? watchy->uptimeSeconds()
                   : WatchySdk::uptimeSeconds());
    WatchyUi::Screen::present(metricBounds);
  }
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderUptimePreview(uint8_t) {
  drawUptime(2 * SECS_PER_DAY + 4 * SECS_PER_HOUR + 18 * SECS_PER_MIN);
}
#endif

} // namespace WatchySensorTools
