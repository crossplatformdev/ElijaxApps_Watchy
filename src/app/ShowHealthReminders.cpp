#include <Watchy.h>
#include "AppDisplay.h"
#include "HealthcareAlerts.h"

namespace {

enum ReminderTool : uint8_t {
  CHECK_IN_TIMER,
  MEDICATION_REMINDER,
  HYDRATION_REMINDER,
  REMINDER_TOOL_COUNT
};

void drawCheckIn(uint16_t minutes, bool armed, int32_t remainingMinutes) {
  char value[12];
  snprintf(value, sizeof(value), "%ldm",
           static_cast<long>(armed ? max<int32_t>(0, remainingMinutes)
                                   : minutes));
  WatchyUi::ValueModel model{
      "CHECK-IN TIMER", value, armed ? "ARMED" : "READY",
      armed ? "SELECT acknowledges. Vibrates every 5m when overdue."
            : "Local vibration only. No message or emergency call.",
      armed ? "SELECT ACKNOWLEDGE    BACK EXIT"
            : "UP/DOWN 5M  SELECT ARM  BACK"};
  WatchyUi::ValueView::draw(model);
  WatchyUi::Screen::present();
}

void runCheckIn(Watchy &watch) {
  WatchyUi::Input::begin();
  uint16_t minutes = 30;
  while (true) {
    watch.RTC.read(watch.currentTime);
    time_t current = makeTime(watch.currentTime);
    bool armed = HealthcareAlerts::checkInArmed();
    int64_t remainingSeconds = armed
                                   ? static_cast<int64_t>(
                                         HealthcareAlerts::checkInDeadline()) -
                                         current
                                   : static_cast<int64_t>(minutes) * 60;
    int32_t remaining = remainingSeconds > 0
                            ? (remainingSeconds + 59) / 60
                            : 0;
    drawCheckIn(minutes, armed, remaining);
    WatchyUi::Event event = WatchyUi::Input::wait();
    if (event == WatchyUi::Event::BACK) {
      return;
    }
    if (event == WatchyUi::Event::SELECT) {
      bool saved = armed
                       ? HealthcareAlerts::acknowledgeCheckIn()
                       : HealthcareAlerts::armCheckIn(
                             current + minutes * SECS_PER_MIN);
      WatchyUi::Feedback::toast(
          saved ? armed ? "CHECK-IN ACKNOWLEDGED" : "CHECK-IN ARMED"
                : "SETTING NOT SAVED");
      delay(700);
      return;
    }
    if (!armed && event == WatchyUi::Event::UP) {
      minutes = WatchyUi::Selector::step(minutes, 5, 5, 240);
    } else if (!armed && event == WatchyUi::Event::DOWN) {
      minutes = WatchyUi::Selector::step(minutes, -5, 5, 240);
    }
  }
}

void drawMedication(const HealthcareAlerts::Configuration &configuration) {
  char value[6];
  WatchyUi::Selector::formatTime(value, configuration.medicationHour,
                                 configuration.medicationMinute);
  WatchyUi::ValueModel model{
      "MEDICATION ALERT", value,
      configuration.medicationEnabled ? "ON" : "OFF",
      "Daily local vibration reminder only.",
      "UP +1H DOWN +5M SELECT TOGGLE"};
  WatchyUi::ValueView::draw(model);
  WatchyUi::Screen::present();
}

void runMedication() {
  WatchyUi::Input::begin();
  HealthcareAlerts::Configuration configuration;
  HealthcareAlerts::load(configuration);
  drawMedication(configuration);
  while (true) {
    WatchyUi::Event event = WatchyUi::Input::wait();
    if (event == WatchyUi::Event::BACK) {
      return;
    }
    HealthcareAlerts::Configuration candidate = configuration;
    if (event == WatchyUi::Event::UP) {
      candidate.medicationHour = WatchyUi::Selector::step(
          candidate.medicationHour, 1, 0, 23, true);
    } else if (event == WatchyUi::Event::DOWN) {
      candidate.medicationMinute = WatchyUi::Selector::step(
          candidate.medicationMinute, 5, 0, 59, true);
    } else if (event == WatchyUi::Event::SELECT) {
      candidate.medicationEnabled = !candidate.medicationEnabled;
    } else {
      continue;
    }
    if (HealthcareAlerts::saveMedication(candidate.medicationEnabled,
                                         candidate.medicationHour,
                                         candidate.medicationMinute)) {
      configuration = candidate;
    } else {
      WatchyUi::Feedback::toast("SETTING NOT SAVED");
      delay(700);
    }
    drawMedication(configuration);
  }
}

void drawHydration(const HealthcareAlerts::Configuration &configuration) {
  char value[8];
  snprintf(value, sizeof(value), "%um", configuration.hydrationMinutes);
  WatchyUi::ValueModel model{
      "HYDRATION ALERT", value,
      configuration.hydrationEnabled ? "ON" : "OFF",
      "Recurring local vibration reminder only.",
      "UP +15M DOWN -15M SELECT TOGGLE"};
  WatchyUi::ValueView::draw(model);
  WatchyUi::Screen::present();
}

void runHydration() {
  WatchyUi::Input::begin();
  HealthcareAlerts::Configuration configuration;
  HealthcareAlerts::load(configuration);
  drawHydration(configuration);
  while (true) {
    WatchyUi::Event event = WatchyUi::Input::wait();
    if (event == WatchyUi::Event::BACK) {
      return;
    }
    HealthcareAlerts::Configuration candidate = configuration;
    if (event == WatchyUi::Event::UP) {
      candidate.hydrationMinutes = WatchyUi::Selector::step(
          candidate.hydrationMinutes, 15, 15, 360);
    } else if (event == WatchyUi::Event::DOWN) {
      candidate.hydrationMinutes = WatchyUi::Selector::step(
          candidate.hydrationMinutes, -15, 15, 360);
    } else if (event == WatchyUi::Event::SELECT) {
      candidate.hydrationEnabled = !candidate.hydrationEnabled;
    } else {
      continue;
    }
    if (HealthcareAlerts::saveHydration(candidate.hydrationEnabled,
                                        candidate.hydrationMinutes)) {
      configuration = candidate;
    } else {
      WatchyUi::Feedback::toast("SETTING NOT SAVED");
      delay(700);
    }
    drawHydration(configuration);
  }
}

} // namespace

void Watchy::showHealthReminder(uint8_t tool) {
  switch (tool) {
  case CHECK_IN_TIMER: runCheckIn(*this); break;
  case MEDICATION_REMINDER: runMedication(); break;
  case HYDRATION_REMINDER: runHydration(); break;
  default: return;
  }
  showMenu(menuIndex, false);
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
namespace WatchyDemo {

void renderHealthReminderPreview(uint8_t tool) {
  const HealthcareAlerts::Configuration configuration{
      true, 8, 30, true, 90};
  switch (tool) {
  case CHECK_IN_TIMER: drawCheckIn(30, true, 18); break;
  case MEDICATION_REMINDER: drawMedication(configuration); break;
  case HYDRATION_REMINDER: drawHydration(configuration); break;
  default: break;
  }
}

} // namespace WatchyDemo
#endif