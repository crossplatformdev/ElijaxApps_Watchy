#ifndef WATCHY_TIME_TOOL_APPS_H
#define WATCHY_TIME_TOOL_APPS_H

#include <Arduino.h>

class Watchy;

namespace WatchyTimeTools {

enum TimerTool : uint8_t {
  STOPWATCH,
  COUNTDOWN,
  DAILY_ALARM,
  POMODORO,
  INTERVAL_TIMER,
  METRONOME,
  TIMER_TOOL_COUNT
};

void runStopwatch();
void runCountdown();
void runDailyAlarm();
void runPomodoro();
void runIntervals();
void runMetronome();

#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderStopwatchPreview(uint8_t view);
void renderCountdownPreview(uint8_t view);
void renderDailyAlarmPreview(uint8_t view);
void renderPomodoroPreview(uint8_t view);
void renderIntervalTimerPreview(uint8_t view);
void renderMetronomePreview(uint8_t view);
#endif

} // namespace WatchyTimeTools

#endif