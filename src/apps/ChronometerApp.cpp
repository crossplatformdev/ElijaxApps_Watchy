#include "../watchy/Watchy.h"
#include "../sdk/UiSDK.h"
#include "../sdk/UiTemplates.h"

bool running = false;
uint32_t savedMs = 0;
uint32_t startMs = millis();
uint32_t nextDraw = 0;
uint32_t nowMs = millis();
uint32_t elapsed = savedMs + (running ? (nowMs - startMs) : 0);

static String formatChrono(uint32_t ms) {
  uint32_t centis = (ms / 10) % 100;
  uint32_t secs   = (ms / 1000) % 60;
  uint32_t mins   = (ms / 60000) % 60;
  uint32_t hours  = ms / 3600000;
  char buf[16];
  snprintf(buf, sizeof(buf), "%02u:%02u:%02u.%02u",
           static_cast<unsigned>(hours), static_cast<unsigned>(mins), static_cast<unsigned>(secs), static_cast<unsigned>(centis));
  return String(buf);
}

static volatile bool sChronoExit = false;

static void chronoBack(Watchy *watchy) {
  
  running = false;
  sChronoExit = true;
}

static void chronoMenu(Watchy *watchy) {
  UiTemplates::waitForAllButtonsReleased(50, 100);
  if (running) {
    savedMs = elapsed;
    running = false;
  } else {
    startMs = millis();
    running = true;
  }
  nextDraw = 0;
}

static void chronoUp(Watchy *watchy) {
  
  running = false;
  savedMs = 0;
  startMs = millis();
  nextDraw = 0;
}

static void chronoDown(Watchy *watchy) {
  watchy->vibMotor(100, 25);
}

UIControlsRowLayout sChronoControls[4] = {
  {"BACK", &Watchy::backPressed},
  {"UP", &Watchy::upPressed},
  {"START/STOP", &Watchy::menuPressed},
  {"BUZZ", &Watchy::downPressed},
};

void Watchy::showChronometer() {
  guiState = APP_STATE;

  UiTemplates::waitForAllButtonsReleased();
  setButtonHandlers(chronoBack, chronoUp, chronoMenu, chronoDown);
  sChronoExit = false;

  while (true) {
    UiSDK::renderControlsRow(*this, sChronoControls);
    if (sChronoExit) {
      clearButtonHandlers();
      showMenu(menuIndex);
      return;
    }

    nowMs = millis();
    elapsed = savedMs + (running ? (nowMs - startMs) : 0);

    if (nowMs >= nextDraw) {
      nextDraw = nowMs + 250;

      RTC.read(currentTime);
      char clockBuf[6];
      snprintf(clockBuf, sizeof(clockBuf), "%02d:%02d", currentTime.Hour, currentTime.Minute);

      UITextSpec header{};
      header.x = 54;
      header.y = 36;
      header.font = &FreeMonoBold9pt7b;
      header.fillBackground = false;
      header.invert = false;
      header.text = "Chronometer";

      UITextSpec nowText{};
      nowText.x = 10;
      nowText.y = 56;
      nowText.font = UiSDK::tinyMono6x8();
      nowText.fillBackground = false;
      nowText.invert = false;
      nowText.text = String("Now ") + clockBuf;

      UITextSpec timeText{};
      timeText.x = 4;
      timeText.y = 120;
      timeText.font = &DSEG7_Classic_Bold_53;
      timeText.fillBackground = false;      
      timeText.invert = false;
      timeText.text = formatChrono(elapsed);

      UITextSpec texts[] = {header, nowText, timeText};

      UIAppSpec app{};
      app.texts = texts;
      app.textCount = 3;
      
      app.controls[0] = sChronoControls[0];
      app.controls[1] = sChronoControls[1];
      app.controls[2] = sChronoControls[2];
      app.controls[3] = sChronoControls[3];
      app.controlCount = 4;
      UiSDK::renderApp(*this, app);       
    }
  }

  delay(100); 
  showMenu(menuIndex);
}
