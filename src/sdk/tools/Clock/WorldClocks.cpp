#include "WatchyUi.h"

#include "AppDisplay.h"
#include "ClockToolApps.h"

namespace WatchyClockTools {

void drawWorldClocks(const tmElements_t &time) {
  time_t utc = currentUtcTime(time);
  const char *const cities[] = {
      "LOS ANGELES", "NEW YORK", "LONDON", "DELHI", "TOKYO", "SYDNEY"};
  const int32_t offsets[] = {
      -8 * SECS_PER_HOUR, -5 * SECS_PER_HOUR, 0,
      5 * SECS_PER_HOUR + 30 * SECS_PER_MIN, 9 * SECS_PER_HOUR,
      10 * SECS_PER_HOUR};
  for (uint8_t index = 0; index < 6; index++) {
    tmElements_t city{};
    char formattedTime[6];
    breakTime(utc + offsets[index], city);
    WatchyUi::Selector::formatTime(formattedTime, city.Hour, city.Minute);
    AppVisual::drawDataRow(43 + index * 20, cities[index], formattedTime,
                           index == 2);
  }
  float localDay = (time.Hour * SECS_PER_HOUR + time.Minute * SECS_PER_MIN) /
                   static_cast<float>(SECS_PER_DAY);
  AppVisual::drawTimeline({8, 168, 184, 8}, 0.25f, 0.75f, localDay);
}

} // namespace WatchyClockTools
