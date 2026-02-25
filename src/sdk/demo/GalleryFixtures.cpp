#include "GalleryFixtures.h"

#include "WatchyUi.h"

#ifdef WATCHY_DETERMINISTIC_GALLERY

namespace WatchyDemo {
namespace GalleryFixtures {

EmergencyProfile::Data emergencyProfile() {
  EmergencyProfile::Data profile;
  profile.name = "ALEX SAMPLE";
  profile.identifier = "DEMO-2026-0142";
  profile.country = "DEMO REPUBLIC";
  profile.birthDate = "1990-04-12";
  profile.bloodType = "O+";
  profile.allergies = "PENICILLIN";
  profile.conditions = "ASTHMA";
  profile.medications = "SALBUTAMOL";
  profile.iceName = "SAM SAMPLE";
  profile.icePhone = "+1-202-555-0142";
  return profile;
}

} // namespace GalleryFixtures
} // namespace WatchyDemo

#endif
