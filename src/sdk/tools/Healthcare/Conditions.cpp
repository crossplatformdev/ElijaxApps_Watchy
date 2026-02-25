#include "HealthcareSupport.h"
#include "HealthcareToolApps.h"

#include "WatchyUi.h"

namespace WatchyHealthcareTools {

void showConditions(const EmergencyProfile::Data &profile) {
  showSingleField("CONDITIONS", "KNOWN CONDITIONS", profile.conditions,
                  "Not a clinical record");
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderConditionsPreview(const EmergencyProfile::Data &profile) {
  drawSingleField("CONDITIONS", "KNOWN CONDITIONS", profile.conditions,
                  "Not a clinical record");
}
#endif

} // namespace WatchyHealthcareTools
