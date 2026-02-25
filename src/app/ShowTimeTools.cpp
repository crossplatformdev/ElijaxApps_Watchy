#include <Watchy.h>
#include <Preferences.h>
#include <stddef.h>
#include "AppDisplay.h"
#include "TimeTools.h"
#include "sdk/WatchyStorage.h"

RTC_DATA_ATTR uint8_t dailyAlarmHour = 7;
RTC_DATA_ATTR uint8_t dailyAlarmMinute = 0;
RTC_DATA_ATTR bool dailyAlarmEnabled = false;
RTC_DATA_ATTR uint32_t dailyAlarmLastDate = 0;
RTC_DATA_ATTR bool dailyAlarmLoaded = false;

namespace {

enum TimerTool : uint8_t {
  STOPWATCH,
  COUNTDOWN,
  DAILY_ALARM,
  POMODORO,
  INTERVAL_TIMER,
  METRONOME,
  TIMER_TOOL_COUNT
};

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

void configureButtons() {
  WatchyUi::Input::begin();
}

void pulseMotor(uint16_t durationMs) {
  Watchy::vibMotor(durationMs, 1);
}

void formatDuration(char *output, size_t outputSize, uint32_t totalSeconds) {
  uint16_t hours = totalSeconds / 3600;
  uint8_t minutes = totalSeconds / 60 % 60;
  uint8_t seconds = totalSeconds % 60;
  if (hours > 0) {
    snprintf(output, outputSize, "%02u:%02u:%02u", hours, minutes, seconds);
  } else {
    snprintf(output, outputSize, "%02u:%02u", minutes, seconds);
  }
}

void drawTimer(const char *title, uint32_t seconds, const char *state,
               const char *controls, const char *detail = nullptr) {
  char value[12];
  formatDuration(value, sizeof(value), seconds);
  WatchyUi::ValueModel model{title, value, state, detail, controls};
  WatchyUi::ValueView::draw(model);
  WatchyUi::Screen::present();
}

void runStopwatch(Watchy &watch) {
  configureButtons();
  bool running = false;
  uint32_t accumulatedMs = 0;
  uint32_t startedAt = 0;
  uint32_t lapMs = 0;
  uint32_t displayedSecond = UINT32_MAX;

  while (true) {
    uint32_t elapsedMs = accumulatedMs + (running ? millis() - startedAt : 0);
    uint32_t elapsedSecond = elapsedMs / 1000;
    if (elapsedSecond != displayedSecond) {
      displayedSecond = elapsedSecond;
      char lapText[24];
      snprintf(lapText, sizeof(lapText), "LAP %02lu:%02lu",
               static_cast<unsigned long>(lapMs / 60000),
               static_cast<unsigned long>(lapMs / 1000 % 60));
      drawTimer("STOPWATCH", elapsedSecond, running ? "RUNNING" : "PAUSED",
                "SELECT START/STOP UP LAP DOWN RESET", lapText);
    }

    WatchyUi::Event event = WatchyUi::Input::poll();
    if (event == WatchyUi::Event::BACK) {
      return;
    }
    if (event == WatchyUi::Event::SELECT) {
      if (running) {
        accumulatedMs += millis() - startedAt;
      } else {
        startedAt = millis();
      }
      running = !running;
      displayedSecond = UINT32_MAX;
    } else if (event == WatchyUi::Event::UP) {
      lapMs = elapsedMs;
      displayedSecond = UINT32_MAX;
    } else if (event == WatchyUi::Event::DOWN) {
      if (!running) {
        accumulatedMs = 0;
        lapMs = 0;
      }
      displayedSecond = UINT32_MAX;
    }
    delay(20);
  }
}

void runCountdown(uint32_t initialSeconds, const char *title, bool pomodoro) {
  configureButtons();
  uint32_t presetSeconds = initialSeconds;
  uint32_t remainingSeconds = presetSeconds;
  uint32_t deadline = 0;
  uint32_t displayedSecond = UINT32_MAX;
  bool running = false;
  bool finished = false;

  while (true) {
    if (running) {
      int32_t remainingMs = static_cast<int32_t>(deadline - millis());
      if (remainingMs <= 0) {
        remainingSeconds = 0;
        running = false;
        finished = true;
        pulseMotor(900);
      } else {
        remainingSeconds = (remainingMs + 999) / 1000;
      }
    }

    if (remainingSeconds != displayedSecond) {
      displayedSecond = remainingSeconds;
      drawTimer(title, remainingSeconds,
                finished ? "FINISHED" : running ? "RUNNING" : "READY",
                pomodoro ? "UP 25M DOWN 5M SELECT START"
                         : "UP +1M DOWN -1M SELECT START");
    }

    WatchyUi::Event event = WatchyUi::Input::poll();
    if (event == WatchyUi::Event::BACK) {
      return;
    }
    if (event == WatchyUi::Event::SELECT) {
      if (finished) {
        remainingSeconds = presetSeconds;
        finished = false;
      } else if (running) {
        running = false;
      } else if (remainingSeconds > 0) {
        deadline = millis() + remainingSeconds * 1000UL;
        running = true;
      }
      displayedSecond = UINT32_MAX;
    } else if (!running && event == WatchyUi::Event::UP) {
      presetSeconds = pomodoro ? 25 * 60UL : min(99 * 60UL, presetSeconds + 60UL);
      remainingSeconds = presetSeconds;
      finished = false;
      displayedSecond = UINT32_MAX;
    } else if (!running && event == WatchyUi::Event::DOWN) {
      presetSeconds = pomodoro ? 5 * 60UL
                               : (presetSeconds > 60 ? presetSeconds - 60 : 60);
      remainingSeconds = presetSeconds;
      finished = false;
      displayedSecond = UINT32_MAX;
    }
    delay(20);
  }
}

void drawAlarm() {
  char value[6];
  WatchyUi::Selector::formatTime(value, dailyAlarmHour, dailyAlarmMinute);
  WatchyUi::ValueModel model{
      "DAILY ALARM", value, dailyAlarmEnabled ? "ON" : "OFF",
      "Daily local vibration alarm.",
      "UP +1H DOWN +5M SELECT TOGGLE"};
  WatchyUi::ValueView::draw(model);
  WatchyUi::Screen::present();
}

void runAlarm() {
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
    } else if (event == WatchyUi::Event::SELECT) {
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

void drawInterval(bool workPhase, uint16_t cycle, uint32_t remaining,
                  bool running) {
  char state[32];
  snprintf(state, sizeof(state), "%s  ROUND %u",
           workPhase ? "WORK" : "REST", cycle);
  drawTimer("INTERVALS", remaining, state,
        running ? "SELECT PAUSE"
          : "SELECT START UP WORK DOWN REST");
}

void runIntervals() {
  configureButtons();
  uint16_t workSeconds = 30;
  uint16_t restSeconds = 10;
  uint16_t cycle = 1;
  bool workPhase = true;
  bool running = false;
  uint32_t remaining = workSeconds;
  uint32_t deadline = 0;
  uint32_t displayed = UINT32_MAX;

  while (true) {
    if (running) {
      int32_t remainingMs = static_cast<int32_t>(deadline - millis());
      if (remainingMs <= 0) {
        pulseMotor(workPhase ? 250 : 500);
        if (!workPhase) {
          cycle++;
        }
        workPhase = !workPhase;
        remaining = workPhase ? workSeconds : restSeconds;
        deadline = millis() + remaining * 1000UL;
      } else {
        remaining = (remainingMs + 999) / 1000;
      }
    }
    if (remaining != displayed) {
      displayed = remaining;
      drawInterval(workPhase, cycle, remaining, running);
    }
    WatchyUi::Event event = WatchyUi::Input::poll();
    if (event == WatchyUi::Event::BACK) {
      return;
    }
    if (event == WatchyUi::Event::SELECT) {
      if (running) {
        running = false;
      } else {
        deadline = millis() + remaining * 1000UL;
        running = true;
      }
      displayed = UINT32_MAX;
    } else if (!running && event == WatchyUi::Event::UP) {
      workSeconds = min<uint16_t>(300, workSeconds + 5);
      if (workPhase) {
        remaining = workSeconds;
      }
      displayed = UINT32_MAX;
    } else if (!running && event == WatchyUi::Event::DOWN) {
      restSeconds = min<uint16_t>(120, restSeconds + 5);
      if (!workPhase) {
        remaining = restSeconds;
      }
      displayed = UINT32_MAX;
    }
    delay(20);
  }
}

void drawMetronome(uint16_t bpm, bool running) {
  char value[8];
  snprintf(value, sizeof(value), "%u", bpm);
  WatchyUi::ValueModel model{
      "METRONOME", value, running ? "RUNNING" : "PAUSED", "BPM",
      "UP/DOWN 5  SELECT START/STOP"};
  WatchyUi::ValueView::draw(model);
  WatchyUi::Screen::present();
}

void runMetronome() {
  configureButtons();
  uint16_t bpm = 100;
  bool running = false;
  uint32_t nextBeat = 0;
  drawMetronome(bpm, running);
  while (true) {
    if (running && static_cast<int32_t>(millis() - nextBeat) >= 0) {
      pulseMotor(35);
      nextBeat += 60000UL / bpm;
    }
    WatchyUi::Event event = WatchyUi::Input::poll();
    if (event == WatchyUi::Event::BACK) {
      return;
    }
    if (event == WatchyUi::Event::SELECT) {
      running = !running;
      nextBeat = millis();
      drawMetronome(bpm, running);
    } else if (event == WatchyUi::Event::UP) {
      bpm = min<uint16_t>(240, bpm + 5);
      nextBeat = millis();
      drawMetronome(bpm, running);
    } else if (event == WatchyUi::Event::DOWN) {
      bpm = max<uint16_t>(30, bpm - 5);
      nextBeat = millis();
      drawMetronome(bpm, running);
    }
    delay(5);
  }
}

} // namespace

void checkDailyAlarm(Watchy &watch, const tmElements_t &now) {
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
  watch.vibMotor(140, 14);
}

void Watchy::showTimerTool(uint8_t tool) {
  switch (tool) {
  case STOPWATCH: runStopwatch(*this); break;
  case COUNTDOWN: runCountdown(5 * 60UL, "COUNTDOWN", false); break;
  case DAILY_ALARM: runAlarm(); break;
  case POMODORO: runCountdown(25 * 60UL, "POMODORO", true); break;
  case INTERVAL_TIMER: runIntervals(); break;
  case METRONOME: runMetronome(); break;
  default: return;
  }
  showMenu(menuIndex, false);
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
namespace WatchyDemo {

void renderTimeToolPreview(uint8_t tool) {
  switch (tool) {
  case STOPWATCH:
    drawTimer("STOPWATCH", 12 * 60 + 34, "RUNNING",
              "SELECT START/STOP UP LAP DOWN RESET", "LAP 04:21");
    break;
  case COUNTDOWN:
    drawTimer("COUNTDOWN", 4 * 60 + 12, "RUNNING",
              "UP +1M DOWN -1M SELECT START");
    break;
  case DAILY_ALARM: {
    WatchyUi::ValueModel model{
        "DAILY ALARM", "07:30", "ON", "Daily local vibration alarm.",
        "UP +1H DOWN +5M SELECT TOGGLE"};
    WatchyUi::ValueView::draw(model);
    WatchyUi::Screen::present();
    break;
  }
  case POMODORO:
    drawTimer("POMODORO", 18 * 60 + 42, "FOCUS",
              "UP 25M DOWN 5M SELECT START");
    break;
  case INTERVAL_TIMER:
    drawInterval(true, 3, 23, true);
    break;
  case METRONOME:
    drawMetronome(120, true);
    break;
  default:
    break;
  }
}

} // namespace WatchyDemo
#endif