#ifndef WATCHY_HEALTHCARE_TOOL_APPS_H
#define WATCHY_HEALTHCARE_TOOL_APPS_H

#include <Arduino.h>

#include "EmergencyProfile.h"

namespace WatchyHealthcareTools {

enum Tool : uint8_t {
  EMERGENCY_PLATE,
  MEDICAL_ID,
  ICE_CONTACT,
  BLOOD_TYPE,
  ALLERGIES,
  MEDICATIONS,
  CONDITIONS,
  EDIT_MEDICAL_ID,
  HYDRATION,
  HEALTHCARE_TOOL_COUNT
};

void drawEmergencyPlate(const EmergencyProfile::Data &profile);
void showMedicalId(const EmergencyProfile::Data &profile);
void drawIceContact(const EmergencyProfile::Data &profile);
void drawBloodType(const EmergencyProfile::Data &profile);
void showAllergies(const EmergencyProfile::Data &profile);
void showMedications(const EmergencyProfile::Data &profile);
void showConditions(const EmergencyProfile::Data &profile);
void showHydration(const EmergencyProfile::Data &profile);
void editMedicalId();

#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderMedicalIdPreview(const EmergencyProfile::Data &profile);
void renderAllergiesPreview(const EmergencyProfile::Data &profile);
void renderMedicationsPreview(const EmergencyProfile::Data &profile);
void renderConditionsPreview(const EmergencyProfile::Data &profile);
void renderEditMedicalIdPreview(const EmergencyProfile::Data &profile,
                                uint8_t view);
void renderHydrationPreview(const EmergencyProfile::Data &profile);
#endif

} // namespace WatchyHealthcareTools

#endif