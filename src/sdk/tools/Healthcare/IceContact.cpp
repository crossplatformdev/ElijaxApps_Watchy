#include "WatchyUi.h"

#include "AppDisplay.h"
#include "HealthcareSupport.h"
#include "HealthcareToolApps.h"

namespace WatchyHealthcareTools {

void drawIceContact(const EmergencyProfile::Data &profile) {
  beginAppDisplay("ICE CONTACT");
  if (profile.iceName == "NOT SET" || profile.icePhone == "NOT SET") {
    AppVisual::drawEmptyState({8, 40, 184, 118}, "NO ICE CONTACT",
                              "Add a contact in Medical ID");
  } else {
    AppVisual::drawStatusIcon({79, 36, 42, 42}, AppVisual::StatusIcon::INFO,
                              true);
    WatchyUi::Canvas::centeredText({0, 88, 200, 17}, "IN CASE OF EMERGENCY", 1,
                                   WatchyUi::Theme::foreground());
    AppVisual::drawDataRow(125, "CONTACT", clipped(profile.iceName, 18).c_str(),
                           true);
    AppVisual::drawDataRow(151, "PHONE", clipped(profile.icePhone, 18).c_str());
    AppVisual::drawDataRow(177, "CALLING", "Use a nearby phone");
  }
  WatchyUi::Widget::footer("WATCHY CANNOT PLACE CALLS");
  finishAppDisplay();
}

} // namespace WatchyHealthcareTools
