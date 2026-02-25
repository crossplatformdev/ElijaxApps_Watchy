#include "WatchyUi.h"
#include "AppDisplay.h"
#include "Watchy.h"

extern RTC_DATA_ATTR bool WIFI_CONFIGURED;
extern RTC_DATA_ATTR long gmtOffset;
extern RTC_DATA_ATTR char lastSSID[33];
extern RTC_DATA_ATTR uint32_t lastIPAddress;

namespace {

void drawAbout(const char *revision, const char *battery,
               const char *network) {
  WatchyUi::Screen::beginCanvas();
  Watchy::display.setFont();
  Watchy::display.setTextSize(1);
  WatchyUi::Canvas::centeredText({0, 1, DISPLAY_WIDTH, 18}, "ABOUT WATCHY",
                                 1, WatchyUi::Theme::foreground());
  WatchyUi::Widget::separator();
  AppVisual::drawMetric({12, 32, 176, 68}, "WATCHY LIBRARY", WATCHY_LIB_VER);
  AppVisual::drawDataRow(124, "HARDWARE", revision, true);
  AppVisual::drawDataRow(146, "BATTERY", battery);
  AppVisual::drawDataRow(168, "WI-FI", network);
  WatchyUi::Widget::footer("BACK EXIT");
  WatchyUi::Screen::present(APP_STATE);
}

} // namespace

void showAboutImpl() {
  float voltage = Watchy::getBatteryVoltage();
  char revision[12];
  char battery[16];
  snprintf(revision, sizeof(revision), "v%u", Watchy::getBoardRevision());
  snprintf(battery, sizeof(battery), "%.2f V", voltage);
    String network = WIFI_CONFIGURED ? String(lastSSID) : "Not configured";
  drawAbout(revision, battery, network.c_str());
}

void Watchy::showAbout() { showAboutImpl(); }

void WatchySdk::showAbout() { showAboutImpl(); }

#ifdef WATCHY_DETERMINISTIC_GALLERY
namespace WatchyDemo {

void renderAboutPreview(uint8_t view) {
  (void)view;
  drawAbout("v3", "3.82 V", "WATCHY_DEMO");
}

} // namespace WatchyDemo
#endif
