#include "../watchy/Watchy.h"
#include "../sdk/UiSDK.h"
#include "../sdk/UiTemplates.h"

static volatile bool sAccelExitRequested = false;

void accelBack(Watchy *watchy) {
  UiTemplates::waitForAllButtonsReleased(50, 100);
  sAccelExitRequested = true;
}

void Watchy::showAccelerometer() {
  UiSDK::initScreen(display);
  Accel acc;

  unsigned long previousMillis = 0;
  const unsigned long interval = 200;

  guiState = APP_STATE;

  sAccelExitRequested = false;
  //UiTemplates::waitForAllButtonsReleased();
  setButtonHandlers(accelBack, nullptr, nullptr, nullptr);

  UIControlsRowLayout controls[4] = {
      {"BACK", &Watchy::backPressed},
      {"", nullptr},
      {"", nullptr},
      {"", nullptr},
  };

  while (!sAccelExitRequested) {

    unsigned long currentMillis = millis();

    // Poll buttons via the standard controls row (invokes the 4 Watchy callbacks).
    UiSDK::renderControlsRow(*this, controls);

    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;

      // Get acceleration data
      bool res          = sensor.getAccel(acc);
      delay(5);  // Add small delay for I2C stability at lower CPU frequencies
      uint8_t direction = sensor.getDirection();

      UiSDK::initScreen(display);
      UiSDK::renderControlsRow(*this, controls);

      // Build text blocks for the SDK
      UITextSpec textSpecs[2];
      uint8_t textCount = 0;

      String mainBlock;
      String directionLine;

      if (!res) {
        mainBlock = "getAccel FAIL";
      } else {
        mainBlock  = "  X:";
        mainBlock += String(acc.x);
        mainBlock += "\n  Y:";
        mainBlock += String(acc.y);
        mainBlock += "\n  Z:";
        mainBlock += String(acc.z);

        switch (direction) {
        case DIRECTION_DISP_DOWN:
          directionLine = "FACE DOWN";
          break;
        case DIRECTION_DISP_UP:
          directionLine = "FACE UP";
          break;
        case DIRECTION_BOTTOM_EDGE:
          directionLine = "BOTTOM EDGE";
          break;
        case DIRECTION_TOP_EDGE:
          directionLine = "TOP EDGE";
          break;
        case DIRECTION_RIGHT_EDGE:
          directionLine = "RIGHT EDGE";
          break;
        case DIRECTION_LEFT_EDGE:
          directionLine = "LEFT EDGE";
          break;
        default:
          directionLine = "ERROR!!!";
          break;
        }
      }

      // First block: X/Y/Z or failure message starting at (0,30)
      textSpecs[0].x               = 0;
      textSpecs[0].y               = 30;
      textSpecs[0].w               = 0;
      textSpecs[0].h               = 0;
      textSpecs[0].font            = &FreeMonoBold9pt7b;
      textSpecs[0].fillBackground  = false;
      textSpecs[0].text            = mainBlock;
      textSpecs[0].invert          = false;
      textCount                    = 1;

      // Second block: direction line at (30,130) when available
      if (directionLine.length() > 0) {
        textSpecs[1].x               = 30;
        textSpecs[1].y               = 130;
        textSpecs[1].w               = 0;
        textSpecs[1].h               = 0;
        textSpecs[1].font            = &FreeMonoBold9pt7b;
        textSpecs[1].fillBackground  = false;
        textSpecs[1].text            = directionLine;
        textSpecs[1].invert          = false;
        textCount                    = 2;
      }

      for (uint8_t i = 0; i < textCount; ++i) {
        UiSDK::renderText(display, textSpecs[i]);
      }
      display.display(true);
    }

    delay(10);
  }

  clearButtonHandlers();

  showMenu(menuIndex);
}
