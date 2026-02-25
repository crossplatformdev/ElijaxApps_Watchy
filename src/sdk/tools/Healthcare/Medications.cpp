#include "HealthcareSupport.h"
#include "HealthcareToolApps.h"

#include "WatchyUi.h"

namespace WatchyHealthcareTools {

void showMedications(const EmergencyProfile::Data &profile) {
  showSingleField("MEDICATIONS", "CURRENT MEDICATIONS", profile.medications,
                  "Dose details may be incomplete");
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderMedicationsPreview(const EmergencyProfile::Data &profile) {
  drawSingleField("MEDICATIONS", "CURRENT MEDICATIONS", profile.medications,
                  "Dose details may be incomplete");
}
#endif

} // namespace WatchyHealthcareTools
