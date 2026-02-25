#include "../../watchy/Watchy.h"
#include "../../sdk/UiSDK.h"
#include "keen.h"
#include "graphics.h"

#define BACKGROUND_SWITCH_MINUTES 10
#define WEEKDAY_X 19
#define WEEKDAY_Y 12
#define POINTS_X 51
#define POINTS_Y 12
#define LIVES_X 35
#define LIVES_Y 44
#define AMMO_X 115
#define AMMO_Y 44
#define BATTERY_X 165
#define BATTERY_Y 180
#define BATTERY_SEGMENTS 3
#define BATTERY_SEGMENT_WIDTH 7
#define BATTERY_SEGMENT_HEIGHT 11
#define BATTERY_SEGMENT_SPACING 9
#define BATTERY_SWEEP_MAX 13
#define BATTERY_SWEEP_TRANSITION 7
#define BATTERY_SWEEP_STEPS (BATTERY_SWEEP_MAX + (BATTERY_SWEEP_TRANSITION * (BATTERY_SEGMENTS - 1)))
#define BATTERY_VOLTAGE_FULL 4.1
#define BATTERY_VOLTAGE_EMPTY 3.7
#define BATTERY_VOLTAGE_RANGE (BATTERY_VOLTAGE_FULL - BATTERY_VOLTAGE_EMPTY)
#define BATTERY_VOLTAGE_STEP BATTERY_VOLTAGE_RANGE / BATTERY_SWEEP_STEPS

namespace {

void drawBackground(Watchy &watchy, time_t epoch) {
  const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);
  uint8_t background_number = epoch / 60 / BACKGROUND_SWITCH_MINUTES % BACKGROUNDS;
  const unsigned char *background = backgrounds[background_number];
  UiSDK::drawBitmap(watchy.display, 0, 0, background, DISPLAY_WIDTH, DISPLAY_HEIGHT, fgColor);
}

void drawTime(Watchy &watchy) {
  const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);
  UiSDK::setFont(watchy.display, &keen_score_font);
  UiSDK::setTextColor(watchy.display, fgColor);
  UiSDK::setCursor(watchy.display, POINTS_X, POINTS_Y);
  UiSDK::print(watchy.display, ";");
  uint8_t hour = watchy.currentTime.Hour % 12;
  if (hour == 0) {
    hour = 12;
  }
  if (hour < 10) {
    UiSDK::print(watchy.display, ";");
  }
  UiSDK::print(watchy.display, hour);
  UiSDK::print(watchy.display, ":");
  if (watchy.currentTime.Minute < 10) {
    UiSDK::print(watchy.display, 0);
  }
  UiSDK::print(watchy.display, watchy.currentTime.Minute);
}

void drawDate(Watchy &watchy) {
  const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);
  UiSDK::setFont(watchy.display, &keen_score_font);
  UiSDK::setTextColor(watchy.display, fgColor);
  UiSDK::setCursor(watchy.display, LIVES_X, LIVES_Y);
  if (watchy.currentTime.Month < 10) {
    UiSDK::print(watchy.display, ";");
  }
  UiSDK::print(watchy.display, watchy.currentTime.Month);
  UiSDK::setCursor(watchy.display, AMMO_X, AMMO_Y);
  if (watchy.currentTime.Day < 10) {
    UiSDK::print(watchy.display, ";");
  }
  UiSDK::print(watchy.display, watchy.currentTime.Day);
}

void drawWeekday(Watchy &watchy, time_t epoch) {
  const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);
  uint8_t weekday_index = weekday(epoch) - 1;
  const unsigned char *current_weekday = weekdays[weekday_index];
  UiSDK::drawBitmap(watchy.display, WEEKDAY_X, WEEKDAY_Y, current_weekday, WEEKDAY_WIDTH, WEEKDAY_HEIGHT, fgColor);
}

void drawBattery(Watchy &watchy) {
  const uint16_t bgColor = UiSDK::getWatchfaceBg(BASE_POLARITY);
  float voltage = watchy.getBatteryVoltage();
  int bar_index = 0;

  for (; bar_index < BATTERY_SWEEP_STEPS; bar_index++) {
    float voltage_test = BATTERY_VOLTAGE_STEP * bar_index + BATTERY_VOLTAGE_EMPTY;
    if (voltage_test >= voltage) {
      break;
    }
  }

  uint8_t bar_indexes[BATTERY_SEGMENTS];
  bar_indexes[0] = min(bar_index, BATTERY_SWEEP_MAX);
  bar_indexes[1] = max(min(bar_index - BATTERY_SWEEP_TRANSITION, BATTERY_SWEEP_MAX), 0);
  bar_indexes[2] = max(bar_index - BATTERY_SWEEP_TRANSITION * 2, 0);

  for (uint8_t segment = 0; segment < BATTERY_SEGMENTS; segment++) {
    const unsigned char *battery_meter_bar = battery_meter_bars[bar_indexes[segment]];
    uint8_t battery_x = BATTERY_X + (segment * BATTERY_SEGMENT_SPACING);
    UiSDK::drawBitmap(watchy.display, battery_x, BATTERY_Y, battery_meter_bar, BATTERY_SEGMENT_WIDTH, BATTERY_SEGMENT_HEIGHT, bgColor);
  }
}

} // namespace

void showWatchFace_Keen(Watchy &watchy) {
  time_t epoch = makeTime(watchy.currentTime);
  UiSDK::initScreen(watchy.display);
  drawBackground(watchy, epoch);
  drawTime(watchy);
  drawDate(watchy);
  drawWeekday(watchy, epoch);
  drawBattery(watchy);
}
