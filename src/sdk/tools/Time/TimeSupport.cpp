#include "WatchyUi.h"
#include "Watchy.h"

#include "AppDisplay.h"
#include "TimeSupport.h"

namespace WatchyTimeTools {

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
               const char *controls, const char *detail, bool valueOnly,
               float progress) {
  char value[12];
  formatDuration(value, sizeof(value), seconds);
  WatchyUi::ValueModel model{title, value, state, detail, controls, progress};
  WatchyUi::ValueView::draw(model);
  if (valueOnly) {
    WatchyUi::Screen::present(WatchyUi::Bounds{0, 34, DISPLAY_WIDTH, 114});
  } else {
    WatchyUi::Screen::present();
  }
}

void runCountdownTimer(uint32_t initialSeconds, const char *title,
                       bool pomodoro) {
  configureButtons();
  uint32_t presetSeconds = initialSeconds;
  uint32_t remainingSeconds = presetSeconds;
  uint32_t deadline = 0;
  uint32_t displayedSeconds = UINT32_MAX;
  bool running = false;
  bool finished = false;
  bool refreshDue = true;

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

    if (refreshDue || remainingSeconds != displayedSeconds) {
      drawTimer(title, remainingSeconds,
                finished ? "FINISHED" : running ? "RUNNING" : "READY",
                pomodoro ? "UP 25M DOWN 5M SELECT START"
                         : "UP +1M DOWN -1M SELECT START",
                nullptr, !refreshDue,
                presetSeconds == 0 ? 1.0f
                                   : 1.0f - static_cast<float>(remainingSeconds) /
                                                presetSeconds);
      displayedSeconds = remainingSeconds;
      refreshDue = false;
    }

    uint32_t waitMs = UINT32_MAX;
    if (running) {
      int32_t remainingMs = static_cast<int32_t>(deadline - millis());
      if (remainingMs <= 0) {
        waitMs = 0;
      } else {
        waitMs = static_cast<uint32_t>(remainingMs) % 1000;
        if (waitMs == 0) waitMs = 1000;
      }
    }
    WatchyUi::Event event = WatchyUi::Input::wait(waitMs);
    if (event == WatchyUi::Event::BACK) {
      return;
    }
    if (event == WatchyUi::Event::MENU) {
      if (finished) {
        remainingSeconds = presetSeconds;
        finished = false;
      } else if (running) {
        int32_t remainingMs = static_cast<int32_t>(deadline - millis());
        remainingSeconds = remainingMs > 0 ? (remainingMs + 999) / 1000 : 0;
        running = false;
      } else if (remainingSeconds > 0) {
        deadline = millis() + remainingSeconds * 1000UL;
        running = true;
      }
      refreshDue = true;
    } else if (!running && event == WatchyUi::Event::UP) {
      presetSeconds = pomodoro ? 25 * 60UL : min(99 * 60UL, presetSeconds + 60UL);
      remainingSeconds = presetSeconds;
      finished = false;
      refreshDue = true;
    } else if (!running && event == WatchyUi::Event::DOWN) {
      presetSeconds = pomodoro ? 5 * 60UL
                               : (presetSeconds > 60 ? presetSeconds - 60 : 60);
      remainingSeconds = presetSeconds;
      finished = false;
      refreshDue = true;
    }
  }
}

} // namespace WatchyTimeTools
