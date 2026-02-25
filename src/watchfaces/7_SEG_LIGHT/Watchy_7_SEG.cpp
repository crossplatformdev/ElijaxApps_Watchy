#include "../../watchy/Watchy.h"
#include "../../sdk/UiSDK.h"
#include "Seven_Segment10pt7b.h"
#include "DSEG7_Classic_Regular_15.h"
#include "DSEG7_Classic_Bold_25.h"
#include "DSEG7_Classic_Regular_39.h"
#include "icons.h"

namespace {

constexpr uint8_t BATTERY_SEGMENT_WIDTH = 7;
constexpr uint8_t BATTERY_SEGMENT_HEIGHT = 11;
constexpr uint8_t BATTERY_SEGMENT_SPACING = 9;

constexpr uint8_t WEATHER_ICON_WIDTH = 48;
constexpr uint8_t WEATHER_ICON_HEIGHT = 32;

struct BatterySegmentsUserData {
  int8_t level;
};

void drawBatterySegments(WatchyGxDisplay &display, void *userData) {
  const auto *data = static_cast<const BatterySegmentsUserData *>(userData);
  if (data == nullptr) {
    return;
  }

  const uint16_t fg = UiSDK::getWatchfaceFg(UiSDK::getThemePolarity());
  const uint16_t bg = UiSDK::getWatchfaceBg(UiSDK::getThemePolarity());

  UiSDK::fillRect(display, 163, 78, 27, BATTERY_SEGMENT_HEIGHT, bg);

  for (int8_t i = 0; i < data->level; ++i) {
    UiSDK::fillRect(display, 163 + (i * BATTERY_SEGMENT_SPACING), 78, BATTERY_SEGMENT_WIDTH,
                     BATTERY_SEGMENT_HEIGHT, fg);
  }
}

int8_t batteryLevelForVoltage(float vbat) {
  if (vbat > 4.0f) {
    return 3;
  }
  if (vbat > 3.6f && vbat <= 4.0f) {
    return 2;
  }
  if (vbat > 3.20f && vbat <= 3.6f) {
    return 1;
  }
  return 0;
}

const unsigned char *weatherIconFor(int16_t code, bool wifiConfigured) {
  if (!wifiConfigured) {
    return chip;
  }

  if (code > 801) {
    return cloudy;
  }
  if (code == 801) {
    return cloudsun;
  }
  if (code == 800) {
    return sunny;
  }
  if (code >= 700) {
    return atmosphere;
  }
  if (code >= 600) {
    return snow;
  }
  if (code >= 500) {
    return rain;
  }
  if (code >= 300) {
    return drizzle;
  }
  if (code >= 200) {
    return thunderstorm;
  }
  return nullptr;
}

} // namespace

void showWatchFace_7SEGLight(Watchy &watchy) {
  UiSDK::initScreen(watchy.display);

  // Step counter managed by core Watchy
  const uint32_t stepCount = sensor.getCounter();

  const weatherData currentWeather = watchy.getWeatherData();
  const int8_t temperature = currentWeather.temperature;
  const int16_t weatherConditionCode = currentWeather.weatherConditionCode;

  int displayHour;
  if (HOUR_12_24 == 12) {
    displayHour = ((watchy.currentTime.Hour + 11) % 12) + 1;
  } else {
    displayHour = watchy.currentTime.Hour;
  }

  String timeStr;
  if (displayHour < 10) {
    timeStr += "0";
  }
  timeStr += String(displayHour);
  timeStr += ":";
  if (watchy.currentTime.Minute < 10) {
    timeStr += "0";
  }
  timeStr += String(watchy.currentTime.Minute);

  const String dayOfWeek = dayStr(watchy.currentTime.Wday);
  const String month = monthShortStr(watchy.currentTime.Month);

  String dayStr2;
  if (watchy.currentTime.Day < 10) {
    dayStr2 += "0";
  }
  dayStr2 += String(watchy.currentTime.Day);

  const String yearStr = String(tmYearToCalendar(watchy.currentTime.Year));
  const String stepsStr = String(stepCount);
  const String tempStr = String(temperature);

  int16_t x1, y1;
  uint16_t w, h;

  UiSDK::setFont(watchy.display, &Seven_Segment10pt7b);
  UiSDK::getTextBounds(watchy.display, dayOfWeek, 0, 0, &x1, &y1, &w, &h);
  if (watchy.currentTime.Wday == 4) {
    w = static_cast<uint16_t>(w - 5);
  }
  const int16_t dayX = static_cast<int16_t>(85 - w);

  UiSDK::getTextBounds(watchy.display, month, 0, 0, &x1, &y1, &w, &h);
  const int16_t monthX = static_cast<int16_t>(85 - w);

  const GFXfont *tempFont = &DSEG7_Classic_Regular_39;
  int16_t tempX = 0;
  int16_t tempY = 150;
  UiSDK::setFont(watchy.display, tempFont);
  UiSDK::getTextBounds(watchy.display, tempStr, 0, 0, &x1, &y1, &w, &h);
  if (159 - static_cast<int16_t>(w) - x1 > 87) {
    tempX = static_cast<int16_t>(159 - w - x1);
    tempY = 150;
  } else {
    tempFont = &DSEG7_Classic_Bold_25;
    UiSDK::setFont(watchy.display, tempFont);
    UiSDK::getTextBounds(watchy.display, tempStr, 0, 0, &x1, &y1, &w, &h);
    tempX = static_cast<int16_t>(159 - w - x1);
    tempY = 136;
  }

  UITextSpec texts[8];
  uint8_t textCount = 0;
  texts[textCount++] = {5, 58, 0, 0, &DSEG7_Classic_Bold_53, false, false, timeStr};
  texts[textCount++] = {dayX, 85, 0, 0, &Seven_Segment10pt7b, false, false, dayOfWeek};
  texts[textCount++] = {monthX, 110, 0, 0, &Seven_Segment10pt7b, false, false, month};
  texts[textCount++] = {5, 120, 0, 0, &DSEG7_Classic_Bold_25, false, false, dayStr2};
  texts[textCount++] = {5, 150, 0, 0, &DSEG7_Classic_Bold_25, false, false, yearStr};
  texts[textCount++] = {35, 190, 0, 0, &DSEG7_Classic_Bold_25, false, false, stepsStr};
  texts[textCount++] = {tempX, tempY, 0, 0, tempFont, false, false, tempStr};

  UIImageSpec images[12];
  uint8_t imageCount = 0;

  auto addImage = [&](const uint8_t *bitmap, int16_t x, int16_t y, int16_t w,
                      int16_t h) {
    if (imageCount >= (sizeof(images) / sizeof(images[0]))) {
      return;
    }
    UIImageSpec &spec = images[imageCount++];
    spec.bitmap = bitmap;
    spec.x = x;
    spec.y = y;
    spec.w = w;
    spec.h = h;
    spec.fromProgmem = true;
    spec.fillBackground = false;
  };
  addImage(steps, 10, 165, 19, 23);
  addImage(battery, 158, 73, 37, 21);
  addImage(WIFI_CONFIGURED ? wifi : wifioff, 116, 75, 26, 18);
  if (BLE_CONFIGURED) {
    addImage(bluetooth, 100, 73, 13, 21);
  }

#ifdef ARDUINO_ESP32S3_DEV
  if (USB_PLUGGED_IN) {
    addImage(charge, 140, 75, 16, 18);
  }
#endif

  addImage(currentWeather.isMetric ? celsius : fahrenheit, 165, 110, 26, 20);

  if (const unsigned char *weatherIcon = weatherIconFor(weatherConditionCode, WIFI_CONFIGURED)) {
    addImage(weatherIcon, 145, 158, WEATHER_ICON_WIDTH, WEATHER_ICON_HEIGHT);
  }

  BatterySegmentsUserData batteryData{batteryLevelForVoltage(watchy.getBatteryVoltage())};
  const UICallbackSpec callbacks[1] = {{&drawBatterySegments, &batteryData}};

  for (uint8_t i = 0; i < imageCount; ++i) {
    UiSDK::renderImage(watchy.display, images[i]);
  }
  UiSDK::renderCallback(watchy.display, callbacks[0]);
  for (uint8_t i = 0; i < textCount; ++i) {
    UiSDK::renderText(watchy.display, texts[i]);
  }
}