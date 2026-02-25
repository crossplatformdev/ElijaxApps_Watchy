#include "SafetyToolApps.h"

#include "WatchyUi.h"
#include "AppDisplay.h"

namespace WatchySafetyTools {
namespace {

void drawLocation(const String &lat, const String &lon, const String &cityId) {
  beginAppDisplay("SAVED LOCATION");
  if (lat.length() > 0 && lon.length() > 0) {
    AppVisual::drawStatusIcon({79, 36, 42, 42}, AppVisual::StatusIcon::INFO,
                              true);
    WatchyUi::Canvas::centeredText({0, 88, 200, 16}, "CONFIGURED COORDINATES", 1,
                                   WatchyUi::Theme::foreground());
    AppVisual::drawDataRow(125, "LAT", lat.c_str(), true);
    AppVisual::drawDataRow(151, "LON", lon.c_str());
    AppVisual::drawDataRow(177, "SOURCE", "Static configuration");
  } else if (cityId.length() > 0) {
    AppVisual::drawStatusIcon({79, 36, 42, 42}, AppVisual::StatusIcon::INFO,
                              true);
    AppVisual::drawMetric({12, 91, 176, 62}, "WEATHER CITY ID",
                          cityId.c_str());
    AppVisual::drawDataRow(177, "LOCATION", "Coordinates not set");
  } else {
    AppVisual::drawEmptyState({8, 40, 184, 118}, "NO SAVED LOCATION",
                              "Watchy has no GPS");
  }
  WatchyUi::Widget::footer("NOT A LIVE GPS POSITION");
  finishAppDisplay();
}

} // namespace

void runConfiguredLocation(const String &lat, const String &lon,
                           const String &cityId) {
  drawLocation(lat, lon, cityId);
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderConfiguredLocationPreview(uint8_t view) {
  switch (view) {
  case 0:
    drawLocation("51.5074", "-0.1278", "");
    break;
  case 1:
    drawLocation("", "", "LONDON");
    break;
  default:
    drawLocation("", "", "");
    break;
  }
}
#endif

} // namespace WatchySafetyTools
