#include "UtilityToolApps.h"

#include "WatchyUi.h"

#include "AppDisplay.h"

namespace WatchyUtilityTools {
namespace {
void draw() {
  beginAppDisplay("HEAP MONITOR");
  char value[16];
  uint32_t freeHeap = ESP.getFreeHeap();
  uint32_t heapSize = ESP.getHeapSize();
  snprintf(value, sizeof(value), "%lu", static_cast<unsigned long>(freeHeap));
  AppVisual::drawMetric({12, 32, 176, 72}, "FREE HEAP / BYTES", value,
                        heapSize == 0 ? 0.0f : static_cast<float>(freeHeap) / heapSize);
  snprintf(value, sizeof(value), "%lu", static_cast<unsigned long>(ESP.getMinFreeHeap()));
  AppVisual::drawDataRow(125, "MIN FREE", value, true);
  snprintf(value, sizeof(value), "%lu", static_cast<unsigned long>(ESP.getMaxAllocHeap()));
  AppVisual::drawDataRow(145, "MAX BLOCK", value);
  snprintf(value, sizeof(value), "%lu", static_cast<unsigned long>(heapSize));
  AppVisual::drawDataRow(165, "TOTAL", value);
  WatchyUi::Widget::footer("NO PSRAM ON WATCHY V3");
  finishAppDisplay();
}
} // namespace
void runHeapMonitor() { draw(); }
#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderHeapMonitorPreview(uint8_t) {
  beginAppDisplay("HEAP MONITOR");
  AppVisual::drawMetric({12, 32, 176, 72}, "FREE HEAP / BYTES", "216384",
                        216384.0f / 327680.0f);
  AppVisual::drawDataRow(125, "MIN FREE", "201728", true);
  AppVisual::drawDataRow(145, "MAX BLOCK", "110592");
  AppVisual::drawDataRow(165, "TOTAL", "327680");
  WatchyUi::Widget::footer("NO PSRAM ON WATCHY V3");
  finishAppDisplay();
}
#endif
} // namespace WatchyUtilityTools
