#include "HealthcareSupport.h"
#include "HealthcareToolApps.h"

#include "WatchyUi.h"

namespace WatchyHealthcareTools {

void showAllergies(const EmergencyProfile::Data &profile) {
  showSingleField("ALLERGIES", "RECORDED ALLERGIES", profile.allergies,
                  "Verify with patient");
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderAllergiesPreview(const EmergencyProfile::Data &profile) {
  drawSingleField("ALLERGIES", "RECORDED ALLERGIES", profile.allergies,
                  "Verify with patient");
}
#endif

} // namespace WatchyHealthcareTools
