#include "HealthcareAlerts.h"
#include <Preferences.h>
#include <stddef.h>
#include "sdk/WatchyStorage.h"

namespace HealthcareAlerts {
namespace {

RTC_DATA_ATTR bool loaded = false;
RTC_DATA_ATTR bool medicationEnabled = false;
RTC_DATA_ATTR uint8_t medicationHour = 8;
RTC_DATA_ATTR uint8_t medicationMinute = 0;
RTC_DATA_ATTR uint32_t medicationLastDate = 0;
RTC_DATA_ATTR bool hydrationEnabled = false;
RTC_DATA_ATTR uint16_t hydrationMinutes = 60;
RTC_DATA_ATTR time_t nextHydrationAt = 0;
RTC_DATA_ATTR bool checkInIsArmed = false;
RTC_DATA_ATTR time_t checkInDueAt = 0;
RTC_DATA_ATTR time_t nextCheckInBuzzAt = 0;

constexpr const char *storageNamespace = "watchy-health";
constexpr const char *recordKey = "alerts";
constexpr uint32_t recordMagic = 0x48544c41UL;
constexpr uint8_t recordVersion = 1;

constexpr uint8_t medicationFlag = 1U << 0;
constexpr uint8_t hydrationFlag = 1U << 1;
constexpr uint8_t checkInFlag = 1U << 2;

struct AlertsRecord {
  uint32_t magic;
  uint16_t size;
  uint8_t version;
  uint8_t flags;
  uint64_t checkInDue;
  uint16_t hydrationMinutes;
  uint8_t medicationHour;
  uint8_t medicationMinute;
  uint32_t checksum;
};

static_assert(sizeof(AlertsRecord) == 24,
              "Healthcare record layout changed");

void updateChecksum(AlertsRecord &record) {
  record.checksum = WatchySdk::recordChecksum(
      &record, offsetof(AlertsRecord, checksum));
}

bool validRecord(const AlertsRecord &record) {
  constexpr uint8_t knownFlags = medicationFlag | hydrationFlag | checkInFlag;
  return record.magic == recordMagic && record.size == sizeof(record) &&
         record.version == recordVersion && !(record.flags & ~knownFlags) &&
         record.medicationHour < 24 && record.medicationMinute < 60 &&
         record.hydrationMinutes >= 15 && record.hydrationMinutes <= 360 &&
         (!(record.flags & checkInFlag) ||
          (record.checkInDue > 0 && record.checkInDue <= UINT32_MAX)) &&
         record.checksum == WatchySdk::recordChecksum(
             &record, offsetof(AlertsRecord, checksum));
}

AlertsRecord currentRecord() {
  AlertsRecord record{};
  record.magic = recordMagic;
  record.size = sizeof(record);
  record.version = recordVersion;
  record.flags = (medicationEnabled ? medicationFlag : 0) |
                 (hydrationEnabled ? hydrationFlag : 0) |
                 (checkInIsArmed ? checkInFlag : 0);
  record.checkInDue = checkInDueAt > 0
                          ? static_cast<uint64_t>(checkInDueAt)
                          : 0;
  record.hydrationMinutes = hydrationMinutes;
  record.medicationHour = medicationHour;
  record.medicationMinute = medicationMinute;
  updateChecksum(record);
  return record;
}

void applyRecord(const AlertsRecord &record) {
  medicationEnabled = record.flags & medicationFlag;
  medicationHour = record.medicationHour;
  medicationMinute = record.medicationMinute;
  hydrationEnabled = record.flags & hydrationFlag;
  hydrationMinutes = record.hydrationMinutes;
  checkInIsArmed = record.flags & checkInFlag;
  checkInDueAt = checkInIsArmed
                     ? static_cast<time_t>(record.checkInDue)
                     : 0;
  nextCheckInBuzzAt = checkInDueAt;
}

bool writeRecord(AlertsRecord &record) {
  updateChecksum(record);
  return WatchySdk::Storage::write(storageNamespace, recordKey,
                                  &record, sizeof(record));
}

void ensureLoaded() {
  if (loaded) {
    return;
  }
  AlertsRecord record{};
  if (WatchySdk::Storage::read(storageNamespace, recordKey,
                              &record, sizeof(record)) &&
      validRecord(record)) {
    applyRecord(record);
    loaded = true;
    return;
  }

  Preferences preferences;
  if (preferences.begin(storageNamespace, true)) {
    medicationEnabled = preferences.getBool("medOn", false);
    medicationHour = min<uint8_t>(preferences.getUChar("medHour", 8), 23);
    medicationMinute = min<uint8_t>(preferences.getUChar("medMinute", 0), 59);
    hydrationEnabled = preferences.getBool("waterOn", false);
    hydrationMinutes = constrain(preferences.getUShort("waterMin", 60),
                                 15, 360);
    uint64_t legacyDue = preferences.getULong64("checkDue", 0);
    checkInIsArmed = preferences.getBool("checkOn", false) &&
                     legacyDue > 0 && legacyDue <= UINT32_MAX;
    checkInDueAt = checkInIsArmed ? static_cast<time_t>(legacyDue) : 0;
    nextCheckInBuzzAt = checkInDueAt;
    preferences.end();
  }
  record = currentRecord();
  writeRecord(record);
  loaded = true;
}

uint32_t dateKey(const tmElements_t &now) {
  return static_cast<uint32_t>(tmYearToCalendar(now.Year)) * 10000UL +
         now.Month * 100UL + now.Day;
}

} // namespace

void load(Configuration &configuration) {
  ensureLoaded();
  configuration = {medicationEnabled, medicationHour, medicationMinute,
                   hydrationEnabled, hydrationMinutes};
}

bool saveMedication(bool enabled, uint8_t hour, uint8_t minute) {
  ensureLoaded();
  AlertsRecord record = currentRecord();
  record.medicationHour = hour % 24;
  record.medicationMinute = minute % 60;
  if (enabled) record.flags |= medicationFlag;
  else record.flags &= ~medicationFlag;
  if (!writeRecord(record)) {
    return false;
  }
  medicationEnabled = enabled;
  medicationHour = record.medicationHour;
  medicationMinute = record.medicationMinute;
  medicationLastDate = 0;
  return true;
}

bool saveHydration(bool enabled, uint16_t intervalMinutes) {
  ensureLoaded();
  AlertsRecord record = currentRecord();
  record.hydrationMinutes = constrain(intervalMinutes, 15, 360);
  if (enabled) record.flags |= hydrationFlag;
  else record.flags &= ~hydrationFlag;
  if (!writeRecord(record)) {
    return false;
  }
  hydrationEnabled = enabled;
  hydrationMinutes = record.hydrationMinutes;
  nextHydrationAt = 0;
  return true;
}

bool armCheckIn(time_t deadline) {
  ensureLoaded();
  if (deadline <= 0 || static_cast<uint64_t>(deadline) > UINT32_MAX) {
    return false;
  }
  AlertsRecord record = currentRecord();
  record.flags |= checkInFlag;
  record.checkInDue = static_cast<uint64_t>(deadline);
  if (!writeRecord(record)) {
    return false;
  }
  checkInIsArmed = true;
  checkInDueAt = deadline;
  nextCheckInBuzzAt = deadline;
  return true;
}

bool acknowledgeCheckIn() {
  ensureLoaded();
  AlertsRecord record = currentRecord();
  record.flags &= ~checkInFlag;
  record.checkInDue = 0;
  if (!writeRecord(record)) {
    return false;
  }
  checkInIsArmed = false;
  checkInDueAt = 0;
  nextCheckInBuzzAt = 0;
  return true;
}

bool checkInArmed() {
  ensureLoaded();
  return checkInIsArmed;
}

time_t checkInDeadline() {
  ensureLoaded();
  return checkInDueAt;
}

void check(Watchy &watch, const tmElements_t &now) {
  ensureLoaded();
  time_t current = makeTime(now);
  uint32_t today = dateKey(now);

  if (medicationEnabled && now.Hour == medicationHour &&
      now.Minute == medicationMinute && medicationLastDate != today) {
    medicationLastDate = today;
    watch.vibMotor(140, 12);
  }

  if (hydrationEnabled) {
    if (nextHydrationAt == 0) {
      nextHydrationAt = current + hydrationMinutes * SECS_PER_MIN;
    } else if (current >= nextHydrationAt) {
      watch.vibMotor(90, 6);
      nextHydrationAt = current + hydrationMinutes * SECS_PER_MIN;
    }
  } else {
    nextHydrationAt = 0;
  }

  if (checkInIsArmed && current >= checkInDueAt &&
      current >= nextCheckInBuzzAt) {
    watch.vibMotor(150, 14);
    nextCheckInBuzzAt = current + 5 * SECS_PER_MIN;
  }
}

} // namespace HealthcareAlerts