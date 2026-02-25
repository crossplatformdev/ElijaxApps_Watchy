#include <Watchy.h>
#include "sdk/WatchyUi.h"

void Watchy::showAccelerometer() {
  display.setFullWindow();
  display.fillScreen(DARKMODE ? GxEPD_BLACK : GxEPD_WHITE);
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);;

  Accel acc;

  long previousMillis = 0;
  long interval       = 200;

  guiState = APP_STATE;

  WatchyUi::Input::begin();

  while (1) {

    unsigned long currentMillis = millis();

    if (WatchyUi::Input::poll() == WatchyUi::Event::BACK) {
      break;
    }

    if (currentMillis - previousMillis > interval) {
      previousMillis = currentMillis;
      // Get acceleration data
      bool res          = sensor.getAccel(acc);
      uint8_t direction = sensor.getDirection();
      display.fillScreen(DARKMODE ? GxEPD_BLACK : GxEPD_WHITE);

      display.setCursor(0, 30);
      if (res == false) {
        display.println("getAccel FAIL");
      } else {
        display.print("  X:");
        display.println(acc.x);
        display.print("  Y:");
        display.println(acc.y);
        display.print("  Z:");
        display.println(acc.z);

        display.setCursor(30, 130);
        switch (direction) {
        case DIRECTION_DISP_DOWN:
          display.println("FACE DOWN");
          break;
        case DIRECTION_DISP_UP:
          display.println("FACE UP");
          break;
        case DIRECTION_BOTTOM_EDGE:
          display.println("BOTTOM EDGE");
          break;
        case DIRECTION_TOP_EDGE:
          display.println("TOP EDGE");
          break;
        case DIRECTION_RIGHT_EDGE:
          display.println("RIGHT EDGE");
          break;
        case DIRECTION_LEFT_EDGE:
          display.println("LEFT EDGE");
          break;
        default:
          display.println("ERROR!!!");
          break;
        }
      }
      display.display(true); // partial refresh
    }
    delay(10);
  }

  showMenu(menuIndex, false);
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
namespace WatchyDemo {

void renderAccelerometerPreview() {
  Watchy::display.setFullWindow();
  Watchy::display.fillScreen(WatchyUi::Theme::background());
  Watchy::display.setFont(&FreeMonoBold9pt7b);
  Watchy::display.setTextColor(WatchyUi::Theme::foreground());
  Watchy::display.setCursor(0, 30);
  Watchy::display.println("  X:184");
  Watchy::display.println("  Y:-92");
  Watchy::display.println("  Z:1002");
  Watchy::display.setCursor(30, 130);
  Watchy::display.println("FACE UP");
  Watchy::display.display(true);
}

} // namespace WatchyDemo
#endif