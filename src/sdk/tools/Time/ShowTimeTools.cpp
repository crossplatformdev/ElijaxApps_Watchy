#include "WatchyUi.h"
#include "Watchy.h"

#include "TimeToolApps.h"

namespace {

void showTimerToolImpl(uint8_t tool, Watchy *watchy) {
  using namespace WatchyTimeTools;

  switch (tool) {
  case STOPWATCH: runStopwatch(); break;
  case COUNTDOWN: runCountdown(); break;
  case DAILY_ALARM: runDailyAlarm(); break;
  case POMODORO: runPomodoro(); break;
  case INTERVAL_TIMER: runIntervals(); break;
  case METRONOME: runMetronome(); break;
  default: break;
  }
  if (watchy != nullptr) {
    watchy->showMenu(menuIndex, false);
  } else {
    WatchySdk::showMenu(menuIndex, false);
  }
}

} // namespace

void Watchy::showTimerTool(uint8_t tool) { showTimerToolImpl(tool, this); }

void WatchySdk::showTimerTool(uint8_t tool) {
  showTimerToolImpl(tool, nullptr);
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
namespace WatchyDemo {

void renderTimeToolPreview(uint8_t tool, uint8_t view) {
  using namespace WatchyTimeTools;

  switch (tool) {
  case STOPWATCH: renderStopwatchPreview(view); break;
  case COUNTDOWN: renderCountdownPreview(view); break;
  case DAILY_ALARM: renderDailyAlarmPreview(view); break;
  case POMODORO: renderPomodoroPreview(view); break;
  case INTERVAL_TIMER: renderIntervalTimerPreview(view); break;
  case METRONOME: renderMetronomePreview(view); break;
  default: break;
  }
}

} // namespace WatchyDemo
#endif
