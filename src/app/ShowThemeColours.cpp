#include <Watchy.h>
#include "sdk/WatchyUi.h"

namespace {

const char *const themeLabels[] = {"Dark", "Light"};

void drawThemeColours(bool darkSelected) {
  uint8_t selected = darkSelected ? 0 : 1;
  WatchyUi::ListView::draw("APPEARANCE", themeLabels, 2, selected,
                           "SELECT APPLY    BACK CANCEL",
                           DARKMODE ? 0 : 1);
  WatchyUi::Screen::present();
}

} // namespace

void Watchy::showThemeColours() {
  WatchyUi::Input::begin();
  bool darkSelected = DARKMODE;
  drawThemeColours(darkSelected);

  while (true) {
    WatchyUi::Event event = WatchyUi::Input::wait();
    if (event == WatchyUi::Event::BACK) {
      showMenu(menuIndex, false);
      return;
    }
    if (event == WatchyUi::Event::UP && !darkSelected) {
      darkSelected = true;
      drawThemeColours(darkSelected);
    } else if (event == WatchyUi::Event::DOWN && darkSelected) {
      darkSelected = false;
      drawThemeColours(darkSelected);
    } else if (event == WatchyUi::Event::SELECT) {
      if (WatchyUi::Theme::setDark(darkSelected)) {
        showMenu(menuIndex, false);
        return;
      }
      WatchyUi::Feedback::toast("SETTING NOT SAVED");
      delay(700);
      drawThemeColours(darkSelected);
    }
  }
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
namespace WatchyDemo {

void renderThemePreview() {
  drawThemeColours(false);
}

} // namespace WatchyDemo
#endif