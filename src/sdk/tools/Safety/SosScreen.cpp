#include "SafetyToolApps.h"

#include "WatchyUi.h"
#include "Watchy.h"

#include "AppDisplay.h"
#include "EmergencyProfile.h"
#include "SafetySupport.h"
#include "WatchyPowerDiagnostics.h"

#ifdef WATCHY_DETERMINISTIC_GALLERY
#include "GalleryFixtures.h"
#endif

namespace WatchySafetyTools {
namespace {

void drawSosScreen(const EmergencyProfile::Data &profile) {
  beginAppDisplay("EMERGENCY");
  AppVisual::drawStatusIcon({79, 32, 42, 42}, AppVisual::StatusIcon::WARNING,
                            true);
  WatchyUi::Canvas::centeredText({0, 80, 200, 34}, "SOS", 5,
                                 WatchyUi::Theme::foreground());
  AppVisual::drawDataRow(128, "NAME", clipped(profile.name).c_str(), true);
  AppVisual::drawDataRow(148, "BLOOD", clipped(profile.bloodType).c_str());
  AppVisual::drawDataRow(168, "ICE", clipped(profile.icePhone).c_str());
  WatchyUi::Widget::footer("CALL LOCAL EMERGENCY  SELECT VIBRATE");
  finishAppDisplay();
}

} // namespace

void runSosScreen() {
  WatchyUi::Input::begin();
  EmergencyProfile::Data profile;
  EmergencyProfile::load(profile);
  drawSosScreen(profile);
  Watchy::vibMotor(120, 12);
  while (true) {
    WatchyUi::Event event = WatchyUi::Input::wait();
    if (event == WatchyUi::Event::BACK) return;
    if (event == WatchyUi::Event::MENU) Watchy::vibMotor(120, 12);
  }
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderSosScreenPreview(uint8_t) {
  EmergencyProfile::Data profile = WatchyDemo::GalleryFixtures::emergencyProfile();
  drawSosScreen(profile);
}
#endif

} // namespace WatchySafetyTools
