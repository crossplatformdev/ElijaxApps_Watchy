#include "EmergencyProfile.h"
#include <Preferences.h>

namespace EmergencyProfile {
namespace {

constexpr const char *storageNamespace = "watchy-med";
const char *const keys[FIELD_COUNT] = {
    "name", "identifier", "country", "birth", "blood",
    "allergies", "conditions", "medications", "iceName", "icePhone"};
const char *const labels[FIELD_COUNT] = {
    "NAME",       "ID / PASSPORT", "COUNTRY",    "BIRTH YYYY-MM-DD",
    "BLOOD TYPE", "ALLERGIES",     "CONDITIONS", "MEDICATIONS",
    "ICE NAME",   "ICE PHONE"};
const size_t limits[FIELD_COUNT] = {28, 24, 16, 10, 5, 64, 64, 64, 28, 24};

String readValue(Preferences &preferences, Field field) {
  String stored = preferences.getString(keys[field], "NOT SET");
  if (stored.length() > limits[field]) {
    stored.remove(limits[field]);
  }
  for (size_t index = 0; index < stored.length(); index++) {
    uint8_t character = static_cast<uint8_t>(stored[index]);
    if (character < 32 || character == 127) {
      stored.setCharAt(index, ' ');
    }
  }
  stored.trim();
  return stored.length() > 0 ? stored : String("NOT SET");
}

} // namespace

void load(Data &profile) {
  Preferences preferences;
  if (!preferences.begin(storageNamespace, true)) {
    profile = {"NOT SET", "NOT SET", "NOT SET", "NOT SET", "NOT SET",
               "NOT SET", "NOT SET", "NOT SET", "NOT SET", "NOT SET"};
    return;
  }
  for (uint8_t index = 0; index < FIELD_COUNT; index++) {
    value(profile, static_cast<Field>(index)) =
        readValue(preferences, static_cast<Field>(index));
  }
  preferences.end();
}

bool save(Field field, const String &newValue) {
  if (field >= FIELD_COUNT || newValue.length() == 0 ||
      newValue.length() > maximumLength(field)) {
    return false;
  }
  Preferences preferences;
  if (!preferences.begin(storageNamespace, false)) {
    return false;
  }
  size_t written = preferences.putString(keys[field], newValue);
  preferences.end();
  return written > 0;
}

String &value(Data &profile, Field field) {
  switch (field) {
  case FIELD_NAME: return profile.name;
  case FIELD_IDENTIFIER: return profile.identifier;
  case FIELD_COUNTRY: return profile.country;
  case FIELD_BIRTH_DATE: return profile.birthDate;
  case FIELD_BLOOD_TYPE: return profile.bloodType;
  case FIELD_ALLERGIES: return profile.allergies;
  case FIELD_CONDITIONS: return profile.conditions;
  case FIELD_MEDICATIONS: return profile.medications;
  case FIELD_ICE_NAME: return profile.iceName;
  case FIELD_ICE_PHONE: return profile.icePhone;
  default: return profile.name;
  }
}

const String &value(const Data &profile, Field field) {
  switch (field) {
  case FIELD_NAME: return profile.name;
  case FIELD_IDENTIFIER: return profile.identifier;
  case FIELD_COUNTRY: return profile.country;
  case FIELD_BIRTH_DATE: return profile.birthDate;
  case FIELD_BLOOD_TYPE: return profile.bloodType;
  case FIELD_ALLERGIES: return profile.allergies;
  case FIELD_CONDITIONS: return profile.conditions;
  case FIELD_MEDICATIONS: return profile.medications;
  case FIELD_ICE_NAME: return profile.iceName;
  case FIELD_ICE_PHONE: return profile.icePhone;
  default: return profile.name;
  }
}

const char *label(Field field) {
  return field < FIELD_COUNT ? labels[field] : "FIELD";
}

size_t maximumLength(Field field) {
  return field < FIELD_COUNT ? limits[field] : 24;
}

} // namespace EmergencyProfile