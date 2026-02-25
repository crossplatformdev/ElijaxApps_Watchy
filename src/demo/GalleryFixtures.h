#ifndef WATCHY_GALLERY_FIXTURES_H
#define WATCHY_GALLERY_FIXTURES_H

#include "app/EmergencyProfile.h"

namespace WatchyDemo {
namespace GalleryFixtures {

#ifdef WATCHY_DETERMINISTIC_GALLERY

EmergencyProfile::Data emergencyProfile();

#endif

} // namespace GalleryFixtures
} // namespace WatchyDemo

#endif