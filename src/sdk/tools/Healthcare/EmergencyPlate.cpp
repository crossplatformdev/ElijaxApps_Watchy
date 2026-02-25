#include "WatchyUi.h"

#include "AppDisplay.h"
#include "HealthcareSupport.h"
#include "HealthcareToolApps.h"

namespace WatchyHealthcareTools {

void drawEmergencyPlate(const EmergencyProfile::Data &profile) {
  beginAppDisplay("UN DOG PLATE");
  AppVisual::drawStatusIcon({12, 34, 37, 37}, AppVisual::StatusIcon::INFO,
                            true);
  WatchyUi::Canvas::centeredText({54, 38, 134, 29},
                                 clipped(profile.name, 18).c_str(), 2,
                                 WatchyUi::Theme::foreground());
  AppVisual::drawDataRow(96, "ID", clipped(profile.identifier, 17).c_str(),
                         true);
  AppVisual::drawDataRow(114, "NATION", clipped(profile.country, 17).c_str());
  AppVisual::drawDataRow(132, "BIRTH", clipped(profile.birthDate, 17).c_str());
  AppVisual::drawDataRow(150, "BLOOD", clipped(profile.bloodType, 17).c_str(),
                         true);
  AppVisual::drawDataRow(168, "ICE", clipped(profile.iceName, 17).c_str());
  AppVisual::drawDataRow(186, "TEL", clipped(profile.icePhone, 17).c_str());
  finishAppDisplay();
}

} // namespace WatchyHealthcareTools
