#include "SensorToolApps.h"

#include "WatchyUi.h"

#include "AppDisplay.h"
#include "SensorManager.h"
#include "SensorSupport.h"

namespace WatchySensorTools {
namespace {

void drawTemperature(float celsius) {
  char value[12];
  char fahrenheit[12];
  snprintf(value, sizeof(value), "%.1f C", celsius);
  snprintf(fahrenheit, sizeof(fahrenheit), "%.1f F", celsius * 1.8f + 32.0f);
  AppVisual::drawMetric({12, 39, 176, 90}, "BMA TEMPERATURE", value,
                        constrain((celsius + 20.0f) / 70.0f, 0.0f, 1.0f),
                        "Internal sensor value");
  AppVisual::drawDataRow(164, "FAHRENHEIT", fahrenheit, true);
}

void draw() {
  drawTemperature(WatchySensor::readTemperature());
}

} // namespace

void runBmaTemperature() {
  runStaticTool("BMA TEMPERATURE", draw);
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderBmaTemperaturePreview(uint8_t) { drawTemperature(28.5f); }
#endif

} // namespace WatchySensorTools
