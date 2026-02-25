#include "../watchy/Watchy.h"
#include "../sdk/UiSDK.h"
#include "../sdk/UiTemplates.h"


RTC_DATA_ATTR uint8_t gAlarmHour;
RTC_DATA_ATTR uint8_t gAlarmMinute;
RTC_DATA_ATTR bool gAlarmEnabled;
RTC_DATA_ATTR uint32_t gAlarmLastKey;

namespace {
// Vibrate in short pulses for up to maxMs, allowing BACK to stop.
static void runInterruptibleBuzz(Watchy &watchy, uint32_t maxMs) {
  const uint32_t startMs = millis();
  uint32_t nextPulseMs = 0;

  // Avoid immediately consuming a carried-over BACK press.
  UiTemplates::waitForAllButtonsReleased();

  while (millis() - startMs < maxMs) {
    if (UiSDK::buttonPressed(BACK_BTN_PIN, 80, true)) {
      break;
    }

    const uint32_t nowMs = millis();
    if (nowMs >= nextPulseMs) {
      watchy.vibMotor(120, 2); // ~240ms total
      nextPulseMs = nowMs + 520;
    }
    delay(10);
  }
}
} // namespace

static uint32_t alarmKey(const tmElements_t &t) {
  uint32_t dayKey = (static_cast<uint32_t>(t.Year) << 9) ^ (static_cast<uint32_t>(t.Month) << 5) ^ static_cast<uint32_t>(t.Day);
  return dayKey * 1440UL + static_cast<uint32_t>(t.Hour) * 60UL + static_cast<uint32_t>(t.Minute);
}

void Watchy::checkAlarmTrigger() {
  if (!gAlarmEnabled) {
    return;
  }

  const uint32_t key = alarmKey(currentTime);
  const uint32_t targetKey = ((static_cast<uint32_t>(currentTime.Year) << 9) ^ (static_cast<uint32_t>(currentTime.Month) << 5) ^ static_cast<uint32_t>(currentTime.Day)) * 1440UL + static_cast<uint32_t>(gAlarmHour) * 60UL + static_cast<uint32_t>(gAlarmMinute);

  if (key == targetKey && gAlarmLastKey != key) {
    // Alert for <=10s, stoppable with BACK.
    runInterruptibleBuzz(*this, 10000UL);
    gAlarmLastKey = key;
  }
}

static void wrapAdjust(uint8_t &value, uint8_t max, int delta) {
  int next = static_cast<int>(value) + delta;
  if (next < 0) {
    next = max;
  } else if (next > max) {
    next = 0;
  }
  value = static_cast<uint8_t>(next);
}

static uint8_t sAlarmHour = 7;
static uint8_t sAlarmMinute = 0;
static bool sAlarmEnabled = false;
static uint8_t sAlarmField = 0; // 0=hour,1=minute,2=enabled
static volatile uint32_t sAlarmNextDraw = 0;
static volatile bool sAlarmExit = false;

static void alarmBack(Watchy *watchy) {
  
  sAlarmExit = true;
}

static void alarmMenu(Watchy *watchy) {
  UiTemplates::waitForAllButtonsReleased(50, 100);
  sAlarmField = (sAlarmField + 1) % 3;
  watchy->vibMotor(40, 2);
  sAlarmNextDraw = 0;
}

static void alarmUp(Watchy *watchy) {
  if (sAlarmField == 0) {
    wrapAdjust(sAlarmHour, 23, +1);
  } else if (sAlarmField == 1) {
    wrapAdjust(sAlarmMinute, 59, +1);
  } else {
    sAlarmEnabled = !sAlarmEnabled;
  }
  watchy->vibMotor(40, 2);
  sAlarmNextDraw = 0;
}

static void alarmDown(Watchy *watchy) {
  if (sAlarmField == 0) {
    wrapAdjust(sAlarmHour, 23, -1);
  } else if (sAlarmField == 1) {
    wrapAdjust(sAlarmMinute, 59, -1);
  } else {
    sAlarmEnabled = !sAlarmEnabled;                                                 
  }
  watchy->vibMotor(40, 2);
  sAlarmNextDraw = 0;
}

UIControlsRowLayout sAlarmControls[4] = {
  {"BACK", &Watchy::backPressed},
  {"UP", &Watchy::upPressed},
  {"NEXT", &Watchy::menuPressed}, // label set at render time
  {"DOWN", &Watchy::downPressed},
};

void Watchy::showAlarm() {
  guiState = APP_STATE;
  UiTemplates::waitForAllButtonsReleased();
  setButtonHandlers(alarmBack, alarmUp, alarmMenu, alarmDown);

  sAlarmField = 0;
  sAlarmNextDraw = 0;
  sAlarmExit = false;

  while (true) {
    const uint32_t nowMs = millis();

    // Pump input frequently so quick taps are not missed.
    UiSDK::renderControlsRow(*this, sAlarmControls);

    if (sAlarmExit) {
      clearButtonHandlers();
      showMenu(menuIndex);
      return;
    }

    if (nowMs >= sAlarmNextDraw) {
      sAlarmNextDraw = nowMs + 300;
      RTC.read(currentTime);

      char nowBuf[16];
      snprintf(nowBuf, sizeof(nowBuf), "%02d:%02d", currentTime.Hour, currentTime.Minute);
      char alarmBuf[8];
      snprintf(alarmBuf, sizeof(alarmBuf), "%02u:%02u", static_cast<unsigned>(sAlarmHour), static_cast<unsigned>(sAlarmMinute));

      UITextSpec title{};
      title.x = 72;
      title.y = 36;
      title.font = &FreeMonoBold9pt7b;
      title.fillBackground = false;
      title.invert = false;
      title.text = "Alarm";

      UITextSpec nowText{};
      nowText.x = 10;
      nowText.y = 56;
      nowText.font = UiSDK::tinyMono6x8();
      nowText.fillBackground = false;
      nowText.invert = false;
      nowText.text = String("Now ") + nowBuf;

      UITextSpec setText{};
      setText.x = 8;
      setText.y = 120;
      setText.font = &DSEG7_Classic_Bold_53;
      setText.fillBackground = false;
      setText.text = alarmBuf;

      UITextSpec status{};
      status.x = 10;
      status.y = 150;
      status.w = WatchyDisplay::WIDTH - 20;
      status.h = 20;
      status.font = &FreeMonoBold9pt7b;      
      status.fillBackground = false;
      status.invert = false;
      status.text = sAlarmEnabled ? "Enabled" : "Disabled";
      if (sAlarmField == 2) {
        status.fillBackground = true;
      }

      UITextSpec fieldHint{};
      fieldHint.x = 10;
      fieldHint.y = 172;
      fieldHint.font = UiSDK::tinyMono6x8();
      fieldHint.fillBackground = false;
      fieldHint.invert = false;
      const char *focus = (sAlarmField == 0) ? "Hour" : (sAlarmField == 1) ? "Minute" : "Enabled";
      fieldHint.text = String("Field: ") + focus;


      UITextSpec texts[] = {title, nowText, setText, status, fieldHint};

      UIAppSpec app{};
      app.texts = texts;
      app.textCount = 5;
      app.controls[0] = sAlarmControls[0];
      app.controls[1] = sAlarmControls[1];
      app.controls[2] = sAlarmControls[2];
      app.controls[3] = sAlarmControls[3];
      app.controlCount = 4;      
      UiSDK::renderApp(*this, app);
    }
  }

  delay(100);
  showMenu(menuIndex);
}
