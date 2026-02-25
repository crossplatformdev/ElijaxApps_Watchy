#include "HealthcareSupport.h"
#include "HealthcareToolApps.h"

#include "WatchyUi.h"

namespace WatchyHealthcareTools {

void drawBloodType(const EmergencyProfile::Data &profile) {
  drawSingleField("BLOOD TYPE", "RECORDED BLOOD GROUP", profile.bloodType,
                  "Confirm before transfusion");
}

} // namespace WatchyHealthcareTools
