#include "HealthcareSupport.h"
#include "HealthcareToolApps.h"

#include "WatchyUi.h"

namespace WatchyHealthcareTools {

void showHydration(const EmergencyProfile::Data &profile) {
  showSingleField("HYDRATION", "HYDRATION STATUS", profile.hydration,
                  "Not a clinical record");
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderHydrationPreview(const EmergencyProfile::Data &profile) {
  drawSingleField("HYDRATION", "HYDRATION STATUS", profile.hydration,
                  "Not a clinical record");
}
#endif

} // namespace WatchyHealthcareTools