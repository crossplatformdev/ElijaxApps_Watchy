#ifndef WATCHY_SAFETY_TOOL_APPS_H
#define WATCHY_SAFETY_TOOL_APPS_H

#include <WatchySdk.h>

namespace WatchySafetyTools {

enum Tool : uint8_t {
  FallDetector,
  BodyPosition,
  ConfiguredLocation,
  SosScreen,
  SosBle,
  ToolCount
};

void runFallDetector();
void runBodyPosition();
void runConfiguredLocation(const String &lat, const String &lon,
                           const String &cityId);
void runSosScreen();
void runSosBle();

#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderFallDetectorPreview(uint8_t view);
void renderBodyPositionPreview(uint8_t view);
void renderConfiguredLocationPreview(uint8_t view);
void renderSosScreenPreview(uint8_t view);
void renderSosBlePreview(uint8_t view);
#endif

} // namespace WatchySafetyTools

#endif