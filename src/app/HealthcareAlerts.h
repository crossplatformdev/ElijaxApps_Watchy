#ifndef HEALTHCARE_ALERTS_H
#define HEALTHCARE_ALERTS_H

#include <Watchy.h>

namespace HealthcareAlerts {

struct Configuration {
  bool medicationEnabled;
  uint8_t medicationHour;
  uint8_t medicationMinute;
  bool hydrationEnabled;
  uint16_t hydrationMinutes;
};

void load(Configuration &configuration);
bool saveMedication(bool enabled, uint8_t hour, uint8_t minute);
bool saveHydration(bool enabled, uint16_t intervalMinutes);
bool armCheckIn(time_t deadline);
bool acknowledgeCheckIn();
bool checkInArmed();
time_t checkInDeadline();
void check(Watchy &watch, const tmElements_t &now);

} // namespace HealthcareAlerts

#endif