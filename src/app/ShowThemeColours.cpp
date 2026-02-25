#include "WatchyUi.h"
#include "Watchy.h"

namespace {

const char *const themeLabels[] = {"Dark", "Light"};

void drawThemeColours(bool darkSelected) {
  uint8_t selected = darkSelected ? 0 : 1;
  WatchyUi::ListView::draw("APPEARANCE", themeLabels, 2, selected,
                           "SELECT APPLY    BACK CANCEL",
                           DARKMODE ? 0 : 1);
  WatchyUi::Screen::present();
}

void updateThemeColours(bool previousDarkSelected, bool darkSelected) {
  WatchyUi::ListView::presentSelectionChange(
      "APPEARANCE", themeLabels, 2, darkSelected ? 0 : 1,
      previousDarkSelected ? 0 : 1,
      "SELECT APPLY    BACK CANCEL", DARKMODE ? 0 : 1);
}

} // namespace

void showThemeColoursImpl(Watchy *watchy) {
  WatchyUi::Input::begin();
  bool darkSelected = DARKMODE;
  drawThemeColours(darkSelected);

  while (true) {
    WatchyUi::Event event = WatchyUi::Input::wait();
    if (event == WatchyUi::Event::BACK) {
      if (watchy != nullptr) {
        watchy->showMenu(menuIndex, false);
      } else {
        WatchySdk::showMenu(menuIndex, false);
      }
      return;
    }
    if (event == WatchyUi::Event::UP && !darkSelected) {
      bool previousDarkSelected = darkSelected;
      darkSelected = true;
      updateThemeColours(previousDarkSelected, darkSelected);
    } else if (event == WatchyUi::Event::DOWN && darkSelected) {
      bool previousDarkSelected = darkSelected;
      darkSelected = false;
      updateThemeColours(previousDarkSelected, darkSelected);
    } else if (event == WatchyUi::Event::MENU) {
      if (WatchyUi::Theme::setDark(darkSelected)) {
        if (watchy != nullptr) {
          watchy->showMenu(menuIndex, false);
        } else {
          WatchySdk::showMenu(menuIndex, false);
        }
        return;
      }
      WatchyUi::Feedback::toast("SETTING NOT SAVED");
      if (WatchyUi::deepSleepDelay(700) ==
          WatchyUi::WakeupReason::BACK_PRESSED) {
        if (watchy != nullptr) {
          watchy->showMenu(menuIndex, false);
        } else {
          WatchySdk::showMenu(menuIndex, false);
        }
        return;
      }
      drawThemeColours(darkSelected);
    }
  }
}

void Watchy::showThemeColours() { showThemeColoursImpl(this); }

void WatchySdk::showThemeColours() { showThemeColoursImpl(nullptr); }

#ifdef WATCHY_DETERMINISTIC_GALLERY
namespace WatchyDemo {

void renderThemePreview(uint8_t view) {
  drawThemeColours(view != 0);
}

} // namespace WatchyDemo
#endif
