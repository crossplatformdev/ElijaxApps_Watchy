#include "WatchyUi.h"
#include "Watchy.h"
#include <Preferences.h>

#include <stddef.h>

#include "TimeSupport.h"
#include "TimeToolApps.h"
#include "TimeTools.h"
#include "WatchyStorage.h"

RTC_DATA_ATTR uint8_t dailyAlarmHour = 7;
RTC_DATA_ATTR uint8_t dailyAlarmMinute = 0;
RTC_DATA_ATTR bool dailyAlarmEnabled = false;
RTC_DATA_ATTR uint32_t dailyAlarmLastDate = 0;
RTC_DATA_ATTR bool dailyAlarmLoaded = false;

namespace {

constexpr const char *alarmNamespace = "watchy-time";
constexpr const char *alarmRecordKey = "alarm";
constexpr uint32_t alarmRecordMagic = 0x4d524c41UL;
constexpr uint8_t alarmRecordVersion = 1;
constexpr uint8_t alarmEnabledFlag = 1U << 0;

struct AlarmRecord {
  uint32_t magic;
  uint16_t size;
  uint8_t version;
  uint8_t flags;
  uint8_t hour;
  uint8_t minute;
  uint8_t reserved[2];
  uint32_t checksum;
};

static_assert(sizeof(AlarmRecord) == 16, "Alarm record layout changed");

void updateAlarmChecksum(AlarmRecord &record) {
  record.checksum = WatchySdk::recordChecksum(
      &record, offsetof(AlarmRecord, checksum));
}

bool validAlarmRecord(const AlarmRecord &record) {
  return record.magic == alarmRecordMagic &&
         record.size == sizeof(record) &&
         record.version == alarmRecordVersion &&
         !(record.flags & ~alarmEnabledFlag) && record.hour < 24 &&
         record.minute < 60 &&
         record.checksum == WatchySdk::recordChecksum(
             &record, offsetof(AlarmRecord, checksum));
}

AlarmRecord makeAlarmRecord(uint8_t hour, uint8_t minute, bool enabled) {
  AlarmRecord record{};
  record.magic = alarmRecordMagic;
  record.size = sizeof(record);
  record.version = alarmRecordVersion;
  record.flags = enabled ? alarmEnabledFlag : 0;
  record.hour = hour;
  record.minute = minute;
  updateAlarmChecksum(record);
  return record;
}

void ensureDailyAlarmLoaded() {
  if (dailyAlarmLoaded) {
    return;
  }
  AlarmRecord record{};
  if (WatchySdk::Storage::read(alarmNamespace, alarmRecordKey,
                              &record, sizeof(record)) &&
      validAlarmRecord(record)) {
    dailyAlarmHour = record.hour;
    dailyAlarmMinute = record.minute;
    dailyAlarmEnabled = record.flags & alarmEnabledFlag;
    dailyAlarmLoaded = true;
    return;
  }

  Preferences preferences;
  if (preferences.begin(alarmNamespace, true)) {
    dailyAlarmHour = min<uint8_t>(preferences.getUChar("alarmHour", 7), 23);
    dailyAlarmMinute = min<uint8_t>(preferences.getUChar("alarmMin", 0), 59);
    dailyAlarmEnabled = preferences.getBool("alarmOn", false);
    preferences.end();
  }
  record = makeAlarmRecord(dailyAlarmHour, dailyAlarmMinute,
                           dailyAlarmEnabled);
  WatchySdk::Storage::write(alarmNamespace, alarmRecordKey,
                            &record, sizeof(record));
  dailyAlarmLoaded = true;
}

bool saveDailyAlarm(uint8_t hour, uint8_t minute, bool enabled) {
  AlarmRecord record = makeAlarmRecord(hour % 24, minute % 60, enabled);
  if (!WatchySdk::Storage::write(alarmNamespace, alarmRecordKey,
                                &record, sizeof(record))) {
    return false;
  }
  dailyAlarmHour = record.hour;
  dailyAlarmMinute = record.minute;
  dailyAlarmEnabled = enabled;
  dailyAlarmLastDate = 0;
  return true;
}

} // namespace

void checkDailyAlarm(const tmElements_t &now) {
  ensureDailyAlarmLoaded();
  if (!dailyAlarmEnabled || now.Hour != dailyAlarmHour ||
      now.Minute != dailyAlarmMinute) {
    return;
  }
  uint32_t date = static_cast<uint32_t>(tmYearToCalendar(now.Year)) * 10000UL +
                  now.Month * 100UL + now.Day;
  if (dailyAlarmLastDate == date) {
    return;
  }
  dailyAlarmLastDate = date;
  Watchy::vibMotor(140, 14);
}

namespace WatchyTimeTools {
namespace {

void drawAlarm() {
  char value[6];
  WatchyUi::Selector::formatTime(value, dailyAlarmHour, dailyAlarmMinute);
  WatchyUi::ValueModel model{
      "DAILY ALARM", value, dailyAlarmEnabled ? "ON" : "OFF",
      "Daily local vibration alarm.",
      "UP +1H DOWN +5M SELECT TOGGLE",
      static_cast<float>(dailyAlarmHour * 60U + dailyAlarmMinute) /
        (24.0f * 60.0f)};
  WatchyUi::ValueView::draw(model);
  WatchyUi::Screen::present();
}

} // namespace

void runDailyAlarm() {
  configureButtons();
  ensureDailyAlarmLoaded();
  drawAlarm();
  while (true) {
    WatchyUi::Event event = WatchyUi::Input::wait();
    if (event == WatchyUi::Event::BACK) {
      return;
    }
    bool saved = true;
    if (event == WatchyUi::Event::UP) {
      saved = saveDailyAlarm((dailyAlarmHour + 1) % 24, dailyAlarmMinute,
                             dailyAlarmEnabled);
    } else if (event == WatchyUi::Event::DOWN) {
      saved = saveDailyAlarm(dailyAlarmHour,
                             (dailyAlarmMinute + 5) % 60,
                             dailyAlarmEnabled);
    } else if (event == WatchyUi::Event::MENU) {
      saved = saveDailyAlarm(dailyAlarmHour, dailyAlarmMinute,
                             !dailyAlarmEnabled);
    } else {
      continue;
    }
    drawAlarm();
    if (!saved) {
      WatchyUi::Feedback::toast("SETTING NOT SAVED");
    }
  }
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderDailyAlarmPreview(uint8_t view) {
  WatchyUi::ValueModel model{
      "DAILY ALARM", "07:30", view == 0 ? "OFF" : "ON",
      "Daily local vibration alarm.",
      "UP +1H DOWN +5M SELECT TOGGLE", 450.0f / (24.0f * 60.0f)};
  WatchyUi::ValueView::draw(model);
  WatchyUi::Screen::present();
}
#endif

} // namespace WatchyTimeTools
