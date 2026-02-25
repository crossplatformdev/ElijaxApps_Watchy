#include "WatchyUi.h"
#include "Watchy.h"

#include "SafetyToolApps.h"

namespace {

void showSafetyToolImpl(uint8_t rawTool, Watchy *watchy) {
  using namespace WatchySafetyTools;
  Tool tool = rawTool < ToolCount ? static_cast<Tool>(rawTool) : FallDetector;
  switch (tool) {
  case FallDetector: runFallDetector(); break;
  case BodyPosition: runBodyPosition(); break;
  case ConfiguredLocation:
    if (watchy != nullptr) {
      runConfiguredLocation(watchy->settings.lat, watchy->settings.lon,
                            watchy->settings.cityID);
    } else {
      runConfiguredLocation(WatchySdk::settings.lat,
                WatchySdk::settings.lon,
                WatchySdk::settings.cityID);
    }
    break;
  case SosScreen: runSosScreen(); break;
  case SosBle: runSosBle(); break;
  default: break;
  }
  if (watchy != nullptr) {
    watchy->showMenu(menuIndex, false);
  } else {
    WatchySdk::showMenu(menuIndex, false);
  }
}

} // namespace

void Watchy::showSafetyTool(uint8_t rawTool) {
  showSafetyToolImpl(rawTool, this);
}

void WatchySdk::showSafetyTool(uint8_t rawTool) {
  showSafetyToolImpl(rawTool, nullptr);
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
namespace WatchyDemo {

void renderSafetyPreview(uint8_t rawTool, uint8_t view) {
  using namespace WatchySafetyTools;
  switch (rawTool < ToolCount ? static_cast<Tool>(rawTool) : FallDetector) {
  case FallDetector: renderFallDetectorPreview(view); break;
  case BodyPosition: renderBodyPositionPreview(view); break;
  case ConfiguredLocation: renderConfiguredLocationPreview(view); break;
  case SosScreen: renderSosScreenPreview(view); break;
  case SosBle: renderSosBlePreview(view); break;
  default: break;
  }
}

} // namespace WatchyDemo
#endif
