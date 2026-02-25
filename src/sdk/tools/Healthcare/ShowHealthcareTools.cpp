#include "WatchyUi.h"
#include "HealthcareToolApps.h"
#include "Watchy.h"

#ifdef WATCHY_DETERMINISTIC_GALLERY
#include "GalleryFixtures.h"
#endif

namespace {

void showHealthcareToolImpl(uint8_t tool, Watchy *watchy) {
  using namespace WatchyHealthcareTools;
  EmergencyProfile::Data profile;
  EmergencyProfile::load(profile);
  switch (tool) {
  case EMERGENCY_PLATE: drawEmergencyPlate(profile); break;
  case MEDICAL_ID: showMedicalId(profile); break;
  case EDIT_MEDICAL_ID: editMedicalId(); break;
  case ICE_CONTACT: drawIceContact(profile); break;
  case BLOOD_TYPE: drawBloodType(profile); break;
  case ALLERGIES: showAllergies(profile); break;
  case MEDICATIONS: showMedications(profile); break;
  case CONDITIONS: showConditions(profile); break;    
  case HYDRATION: showHydration(profile); break;
  default: break;
  }
  if (watchy != nullptr) {
    watchy->showMenu(menuIndex, false);
  } else {
    WatchySdk::showMenu(menuIndex, false);
  }
}

} // namespace

void Watchy::showHealthcareTool(uint8_t tool) {
  showHealthcareToolImpl(tool, this);
}

void WatchySdk::showHealthcareTool(uint8_t tool) {
  showHealthcareToolImpl(tool, nullptr);
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
namespace WatchyDemo {

void renderHealthcarePreview(uint8_t tool, uint8_t view) {
  using namespace WatchyHealthcareTools;

  EmergencyProfile::Data profile = GalleryFixtures::emergencyProfile();
  if (view == 1 && tool >= MEDICAL_ID && tool <= CONDITIONS) {
    profile.name = "NOT SET";
    profile.bloodType = "NOT SET";
    profile.allergies = "NOT SET";
    profile.conditions = "NOT SET";
    profile.medications = "NOT SET";
    profile.iceName = "NOT SET";
    profile.icePhone = "NOT SET";
  }
  switch (tool) {
  case EMERGENCY_PLATE: drawEmergencyPlate(profile); break;
  case MEDICAL_ID: renderMedicalIdPreview(profile); break;
  case ICE_CONTACT: drawIceContact(profile); break;
  case BLOOD_TYPE: drawBloodType(profile); break;
  case ALLERGIES: renderAllergiesPreview(profile); break;
  case MEDICATIONS: renderMedicationsPreview(profile); break;
  case CONDITIONS: renderConditionsPreview(profile); break;
  case EDIT_MEDICAL_ID: renderEditMedicalIdPreview(profile, view); break;
  case HYDRATION: renderHydrationPreview(profile); break;
  default:
    break;
  }
}

} // namespace WatchyDemo
#endif
