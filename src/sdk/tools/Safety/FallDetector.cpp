#include "SafetyToolApps.h"

#include "WatchyUi.h"
#include "Watchy.h"

#include <math.h>

#include "AppDisplay.h"
#include "FallDetection.h"
#include "SafetySupport.h"
#include "SensorManager.h"

namespace WatchySafetyTools {
namespace {

enum class State : uint8_t { Monitoring, LowGravity, Impact, Alert };

void drawTimeline(State state) {
  const uint16_t foreground = WatchyUi::Theme::foreground();
  constexpr int16_t y = 132;
  for (uint8_t step = 0; step < 4; step++) {
    int16_t x = 32 + step * 45;
    if (step > 0) Watchy::display.drawLine(x - 36, y, x - 9, y, foreground);
    Watchy::display.drawCircle(x, y, 7, foreground);
    if (step <= static_cast<uint8_t>(state)) {
      Watchy::display.fillCircle(x, y, 4, foreground);
    }
  }
  Watchy::display.setFont();
  Watchy::display.setTextSize(1);
  Watchy::display.setCursor(16, 151);
  Watchy::display.print("LOW-G");
  Watchy::display.setCursor(66, 151);
  Watchy::display.print("IMPACT");
  Watchy::display.setCursor(124, 151);
  Watchy::display.print("STILL");
  Watchy::display.setCursor(168, 151);
  Watchy::display.print("ALERT");
}

void drawState(State state) {
  beginAppDisplay("FALL DETECTOR");
  const char *headline = "MONITORING";
  const char *detail = "Low-g + impact + stillness";
  switch (state) {
  case State::Monitoring: break;
  case State::LowGravity: headline = "LOW-G EVENT"; detail = "Watching for an impact"; break;
  case State::Impact: headline = "CHECKING STILL"; detail = "Checking movement after impact"; break;
  case State::Alert: headline = "FALL SUSPECTED"; detail = "Check person; call emergency"; break;
  }
  AppVisual::drawStatusIcon({79, 34, 42, 42},
                            state == State::Monitoring
                                ? AppVisual::StatusIcon::SENSOR
                                : AppVisual::StatusIcon::WARNING,
                            true);
  WatchyUi::Canvas::centeredText({0, 85, 200, 19}, headline, 2,
                                 WatchyUi::Theme::foreground());
  WatchyUi::Canvas::centeredText({12, 108, 176, 14}, detail, 1,
                                 WatchyUi::Theme::foreground());
  drawTimeline(state);
  WatchyUi::Widget::footer(state == State::Alert ? "SELECT RESET ALERT  BACK STOP"
                                                 : "ACTIVE ONLY IN APP  BACK STOP");
  finishAppDisplay();
}

void runForeground() {
  WatchyUi::Input::begin();
  if (!WatchySensor::acquireForeground(WatchySensor::Mode::ForegroundFall)) {
    WatchyUi::Feedback::showMessage(
        "FALL DETECTOR", "BMA setup failed. Background mode preserved.",
        WatchyUi::MessageKind::ERROR, "BACK EXIT");
    while (WatchyUi::Input::wait() != WatchyUi::Event::BACK) {}
    return;
  }
  State state = State::Monitoring;
  uint32_t lowGravityStarted = 0;
  uint32_t stateStarted = millis();
  uint32_t stillStarted = 0;
  float previousMagnitude = 1.0f;
  drawState(state);
  while (true) {
    WatchyUi::Event event = WatchyUi::Input::wait(20);
    if (event == WatchyUi::Event::BACK) break;
    if (state == State::Alert && event == WatchyUi::Event::MENU) {
      state = State::Monitoring;
      lowGravityStarted = 0;
      stillStarted = 0;
      drawState(state);
    }
    Accel acceleration{};
    if (!WatchySensor::readAcceleration(acceleration)) continue;
    uint32_t now = millis();
    float magnitude = magnitudeG(acceleration);
    if (state == State::Monitoring) {
      if (magnitude < 0.45f) {
        if (lowGravityStarted == 0) lowGravityStarted = now;
        else if (now - lowGravityStarted >= 120) {
          state = State::LowGravity;
          stateStarted = now;
          drawState(state);
        }
      } else {
        lowGravityStarted = 0;
      }
    } else if (state == State::LowGravity) {
      if (magnitude > 2.5f) {
        state = State::Impact;
        stateStarted = now;
        stillStarted = 0;
        drawState(state);
      } else if (now - stateStarted > 1500) {
        state = State::Monitoring;
        lowGravityStarted = 0;
        drawState(state);
      }
    } else if (state == State::Impact) {
      bool still = fabsf(magnitude - 1.0f) < 0.35f &&
                   fabsf(magnitude - previousMagnitude) < 0.10f;
      if (still) {
        if (stillStarted == 0) stillStarted = now;
        else if (now - stillStarted >= 2500) {
          state = State::Alert;
          drawState(state);
          Watchy::vibMotor(120, 14);
        }
      } else {
        stillStarted = 0;
      }
      if (state == State::Impact && now - stateStarted > 8000) {
        state = State::Monitoring;
        lowGravityStarted = 0;
        drawState(state);
      }
    }
    previousMagnitude = magnitude;
  }
  WatchySensor::releaseForeground(WatchySensor::Mode::ForegroundFall);
}

void drawConfiguration(const FallDetection::Status &status) {
  const char *value = !status.enabled ? "OFF"
                      : status.armed ? "ARMED"
                      : status.traceCount >= status.capacity ? "LOG FULL" : "ERROR";
  char traceStatus[20];
  snprintf(traceStatus, sizeof(traceStatus), "TRACES %u / %u", status.traceCount,
           status.capacity);
  WatchyUi::ValueModel model{"FALL MONITOR", value, traceStatus,
                             "Calibration logging only. No automatic SOS action.",
                             "SEL TOGGLE UP LIVE DN CLEAR"};
  WatchyUi::ValueView::draw(model);
  WatchyUi::Screen::present();
}

} // namespace

void runFallDetector() {
  Serial.begin(115200);
  pinMode(ACC_INT_2_PIN, INPUT);
  WatchyUi::Input::begin();
  drawConfiguration(FallDetection::status());
  while (true) {
    FallDetection::serviceSerial();
    FallDetection::Status status = FallDetection::status();
    if (status.armed && digitalRead(ACC_INT_2_PIN) == ACTIVE_LOW) {
      FallDetection::handleWake();
      drawConfiguration(FallDetection::status());
    }
    WatchyUi::Event event = WatchyUi::Input::wait(100);
    if (event == WatchyUi::Event::BACK) return;
    if (event == WatchyUi::Event::MENU) {
      bool saved = FallDetection::setEnabled(!status.enabled);
      drawConfiguration(FallDetection::status());
      if (!saved) WatchyUi::Feedback::toast("SETTING NOT SAVED");
    } else if (event == WatchyUi::Event::UP) {
      runForeground();
      WatchyUi::Input::begin();
      drawConfiguration(FallDetection::status());
    } else if (event == WatchyUi::Event::DOWN && status.traceCount > 0) {
      if (WatchyUi::Feedback::confirm("CLEAR TRACES",
                                      "Delete all captured fall candidates?")) {
        if (!FallDetection::clearTraces()) {
          WatchyUi::Feedback::toast("TRACES NOT CLEARED");
        }
      }
      drawConfiguration(FallDetection::status());
    }
  }
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderFallDetectorPreview(uint8_t view) {
  if (view <= static_cast<uint8_t>(State::Alert)) {
    drawState(static_cast<State>(view));
  } else {
    drawConfiguration(FallDetection::Status{true, true, 2, FallDetection::traceCapacity});
  }
}
#endif

} // namespace WatchySafetyTools
