#ifndef EMERGENCY_PROFILE_H
#define EMERGENCY_PROFILE_H

#include <Arduino.h>

namespace EmergencyProfile {

struct Data {
  String name;
  String identifier;
  String country;
  String birthDate;
  String bloodType;
  String allergies;
  String conditions;
  String medications;
  String iceName;
  String icePhone;
};

enum Field : uint8_t {
  FIELD_NAME,
  FIELD_IDENTIFIER,
  FIELD_COUNTRY,
  FIELD_BIRTH_DATE,
  FIELD_BLOOD_TYPE,
  FIELD_ALLERGIES,
  FIELD_CONDITIONS,
  FIELD_MEDICATIONS,
  FIELD_ICE_NAME,
  FIELD_ICE_PHONE,
  FIELD_COUNT
};

void load(Data &profile);
bool save(Field field, const String &value);
String &value(Data &profile, Field field);
const String &value(const Data &profile, Field field);
const char *label(Field field);
size_t maximumLength(Field field);

} // namespace EmergencyProfile

#endif