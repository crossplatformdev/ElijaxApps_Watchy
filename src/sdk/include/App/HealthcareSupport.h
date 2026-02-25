#ifndef WATCHY_HEALTHCARE_SUPPORT_H
#define WATCHY_HEALTHCARE_SUPPORT_H

#include <Arduino.h>

#include "EmergencyProfile.h"

namespace WatchyHealthcareTools {

String clipped(const String &value, size_t length);
void drawSingleField(const char *title, const char *label, const String &value,
                     const char *footer);
void showSingleField(const char *title, const char *label, const String &value,
                     const char *footer);
void editProfile();

#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderProfileEditorPreview(const EmergencyProfile::Data &profile,
                                uint8_t view);
#endif

} // namespace WatchyHealthcareTools

#endif