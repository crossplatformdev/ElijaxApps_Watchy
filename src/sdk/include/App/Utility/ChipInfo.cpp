#include "UtilityToolApps.h"

#include "WatchyUi.h"

#include <esp32-hal-cpu.h>
#include <esp_system.h>

#include "AppDisplay.h"

namespace WatchyUtilityTools {
namespace {
void draw() {
  esp_chip_info_t info;
  esp_chip_info(&info);
  beginAppDisplay("CHIP INFO");
  AppVisual::drawMetric({12, 32, 176, 70}, "SYSTEM ON CHIP", "ESP32-S3");
  char value[16];
  snprintf(value, sizeof(value), "%u", info.cores);
  AppVisual::drawDataRow(124, "CORES", value, true);
  snprintf(value, sizeof(value), "%u", info.revision);
  AppVisual::drawDataRow(143, "REVISION", value);
  snprintf(value, sizeof(value), "%u MHz", getCpuFrequencyMhz());
  AppVisual::drawDataRow(162, "CPU", value);
  snprintf(value, sizeof(value), "%u MB", ESP.getFlashChipSize() / 1048576);
  AppVisual::drawDataRow(181, "FLASH", value);
  finishAppDisplay();
}
} // namespace
void runChipInfo() { draw(); }
#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderChipInfoPreview(uint8_t) {
  beginAppDisplay("CHIP INFO");
  AppVisual::drawMetric({12, 32, 176, 70}, "SYSTEM ON CHIP", "ESP32-S3");
  AppVisual::drawDataRow(124, "CORES", "2", true);
  AppVisual::drawDataRow(143, "REVISION", "2");
  AppVisual::drawDataRow(162, "CPU", "40 MHz");
  AppVisual::drawDataRow(181, "FLASH", "8 MB");
  finishAppDisplay();
}
#endif
} // namespace WatchyUtilityTools
