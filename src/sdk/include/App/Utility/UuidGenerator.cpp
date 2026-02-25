#include "UtilityToolApps.h"

#include "WatchyUi.h"

#include <esp_system.h>
#include <string.h>

#include "AppDisplay.h"
#include "UtilitySupport.h"

namespace WatchyUtilityTools {
namespace {
void generate(char *output, size_t) {
  uint8_t bytes[16];
  for (uint8_t index = 0; index < 16; index += 4) {
    uint32_t value = esp_random();
    memcpy(bytes + index, &value, 4);
  }
  bytes[6] = (bytes[6] & 0x0f) | 0x40;
  bytes[8] = (bytes[8] & 0x3f) | 0x80;
  snprintf(output, 40,
           "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-"
           "%02x%02x%02x%02x%02x%02x",
           bytes[0], bytes[1], bytes[2], bytes[3], bytes[4], bytes[5],
           bytes[6], bytes[7], bytes[8], bytes[9], bytes[10], bytes[11],
           bytes[12], bytes[13], bytes[14], bytes[15]);
}
void draw(const char *value) {
  beginAppDisplay("UUID GENERATOR");
  char firstLine[19];
  memcpy(firstLine, value, 18);
  firstLine[18] = '\0';
  AppVisual::drawStatusIcon({79, 36, 42, 42}, AppVisual::StatusIcon::INFO, true);
  AppVisual::drawDataRow(108, "UUID", firstLine, true);
  AppVisual::drawDataRow(132, "", value + 18);
  AppVisual::drawDataRow(161, "FORMAT", "RFC 4122 v4");
  WatchyUi::Widget::footer("SELECT NEW  BACK EXIT");
  finishAppDisplay();
}
} // namespace
void runUuidGenerator() { runGenerator(generate, draw); }
#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderUuidGeneratorPreview(uint8_t) {
  draw("2f7469e8-9f4a-4d73-b156-874dda019e62");
}
#endif
} // namespace WatchyUtilityTools
