#include "WatchyUi.h"
#include "AppDisplay.h"
#include "Watchy.h"

namespace {

void drawBuzz() {
  WatchyUi::Screen::beginCanvas();
  Watchy::display.setFont();
  Watchy::display.setTextSize(1);
  WatchyUi::Canvas::centeredText({0, 1, DISPLAY_WIDTH, 18}, "VIBRATION TEST",
                                 1, WatchyUi::Theme::foreground());
  WatchyUi::Widget::separator();
  AppVisual::drawStatusIcon({79, 38, 42, 42}, AppVisual::StatusIcon::TIME,
                            true);
  WatchyUi::Canvas::centeredText({0, 92, 200, 22}, "TEST PULSE", 2,
                                 WatchyUi::Theme::foreground());
  AppVisual::drawSignalBars({44, 124, 112, 42}, 1, 3, true);
  WatchyUi::Widget::footer("MOTOR TEST COMPLETE");
  WatchyUi::Screen::present(APP_STATE);
}

} // namespace

void showBuzzImpl(Watchy *watchy) {
  drawBuzz();
  Watchy::vibMotor();
  if (watchy != nullptr) {
    watchy->showMenu(menuIndex, false);
  } else {
    WatchySdk::showMenu(menuIndex, false);
  }
}

void Watchy::showBuzz() { showBuzzImpl(this); }

void WatchySdk::showBuzz() { showBuzzImpl(nullptr); }

#ifdef WATCHY_DETERMINISTIC_GALLERY
namespace WatchyDemo {

void renderBuzzPreview(uint8_t view) {
  (void)view;
  drawBuzz();
}

} // namespace WatchyDemo
#endif
