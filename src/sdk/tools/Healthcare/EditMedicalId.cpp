#include "HealthcareSupport.h"
#include "HealthcareToolApps.h"

#include "WatchyUi.h"

namespace WatchyHealthcareTools {

void editMedicalId() {
  editProfile();
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderEditMedicalIdPreview(const EmergencyProfile::Data &profile,
                                uint8_t view) {
  renderProfileEditorPreview(profile, view);
}
#endif

} // namespace WatchyHealthcareTools
