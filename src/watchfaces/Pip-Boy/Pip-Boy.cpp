#include "../../watchy/Watchy.h"
#include "../../sdk/UiSDK.h"
#include "pipboy/monofonto28pt7b.h"
#include "pipboy/monofonto10pt7b.h"
#include "pipboy/monofonto8pt7b.h"
#include "pipboy/monofonto6pt7b.h"
#include "pipboy/img.h"
#include "pipboy/icons.h"

#define STEPSGOAL 5000
const uint8_t WEATHER_ICON_WIDTH = 48;
const uint8_t WEATHER_ICON_HEIGHT = 32;

extern RTC_DATA_ATTR uint8_t vaultBoyNum;

namespace {

void drawTime(Watchy &watchy) {
  const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);

  if (watchy.currentTime.Minute % 15 == 0) {
    vaultBoyNum = random(0, 3);
  }

  switch (vaultBoyNum) {
    case 0:
      UiSDK::drawBitmap(watchy.display, 70, 50, vaultboy, 57, 100, fgColor);
      break;
    case 1:
      UiSDK::drawBitmap(watchy.display, 70, 50, vaultboypoint, 57, 100, fgColor);
      break;
    case 2:
      UiSDK::drawBitmap(watchy.display, 60, 50, vaultboysmile, 67, 100, fgColor);
      break;
  }

  UiSDK::drawLine(watchy.display, 137, 28, 200, 28, fgColor);
  UiSDK::drawLine(watchy.display, 137, 28, 137, 132, fgColor);
  UiSDK::drawLine(watchy.display, 137, 132, 157, 132, fgColor);
  UiSDK::drawLine(watchy.display, 180, 132, 200, 132, fgColor);

  UiSDK::setFont(watchy.display, &monofonto28pt7b);
  UiSDK::setCursor(watchy.display, 141, 75);

  int displayHour;
  if (HOUR_12_24 == 12) {
    displayHour = ((watchy.currentTime.Hour + 11) % 12) + 1;
  } else {
    displayHour = watchy.currentTime.Hour;
  }
  if (displayHour < 10) {
    UiSDK::print(watchy.display, "0");
  }
  UiSDK::print(watchy.display, displayHour);

  UiSDK::setCursor(watchy.display, 141, 125);
  if (watchy.currentTime.Minute < 10) {
    UiSDK::print(watchy.display, "0");
  }
  UiSDK::print(watchy.display, watchy.currentTime.Minute);

  UiSDK::setFont(watchy.display, &monofonto8pt7b);
  UiSDK::setCursor(watchy.display, 160, 140);
  UiSDK::print(watchy.display, watchy.currentTime.Hour < 11 ? "AM" : "PM");
}

void drawDate(Watchy &watchy) {
  const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);
  const uint16_t bgColor = UiSDK::getWatchfaceBg(BASE_POLARITY);

  UiSDK::setFont(watchy.display, &monofonto10pt7b);
  int16_t x1, y1;
  uint16_t w, h;
  String dayOfWeek = dayStr(watchy.currentTime.Wday);
  dayOfWeek.toUpperCase();
  UiSDK::setTextColor(watchy.display, bgColor);
  UiSDK::getTextBounds(watchy.display, dayOfWeek, 7, 42, &x1, &y1, &w, &h);
  UiSDK::setCursor(watchy.display, 7, 42);
  UiSDK::fillRect(watchy.display, x1 - 2, y1 - 2, w + 4, h + 4, fgColor);
  UiSDK::print(watchy.display, dayOfWeek);

  UiSDK::setFont(watchy.display, &monofonto10pt7b);
  UiSDK::setTextColor(watchy.display, fgColor);
  UiSDK::setCursor(watchy.display, 7, 62);
  UiSDK::print(watchy.display, monthShortStr(watchy.currentTime.Month));
  UiSDK::print(watchy.display, " ");
  UiSDK::print(watchy.display, watchy.currentTime.Day);
  UiSDK::setCursor(watchy.display, 7, 78);
  UiSDK::print(watchy.display, tmYearToCalendar(watchy.currentTime.Year));
}

void drawSteps(Watchy &watchy) {
  const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);

  uint32_t stepCount = sensor.getCounter();
  uint8_t progress = (uint8_t)(stepCount * 100.0 / STEPSGOAL);
  progress = progress > 100 ? 100 : progress;
  UiSDK::drawBitmap(watchy.display, 60, 155, gauge, 73, 10, fgColor);
  UiSDK::fillRect(watchy.display, 60 + 13, 155 + 5, (progress / 2) + 5, 4, fgColor);

  UiSDK::setFont(watchy.display, &monofonto8pt7b);
  UiSDK::setTextColor(watchy.display, fgColor);
  UiSDK::setCursor(watchy.display, 150, 160);
  UiSDK::print(watchy.display, "STEPS");
  UiSDK::setCursor(watchy.display, 150, 175);
  UiSDK::print(watchy.display, stepCount);
}

void drawBattery(Watchy &watchy) {
  const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);
  const uint16_t bgColor = UiSDK::getWatchfaceBg(BASE_POLARITY);

  UiSDK::drawBitmap(watchy.display, 10, 150, battery, 37, 21, fgColor);
  UiSDK::fillRect(watchy.display, 15, 155, 27, 11, bgColor);
  int8_t batteryLevel = 0;
  float VBAT = watchy.getBatteryVoltage();
  if (VBAT > 4.1) {
    batteryLevel = 3;
  } else if (VBAT > 3.95 && VBAT <= 4.1) {
    batteryLevel = 2;
  } else if (VBAT > 3.80 && VBAT <= 3.95) {
    batteryLevel = 1;
  } else if (VBAT <= 3.80) {
    batteryLevel = 0;
  }

  for (int8_t batterySegments = 0; batterySegments < batteryLevel; batterySegments++) {
    UiSDK::fillRect(watchy.display, 15 + (batterySegments * 9), 155, 7, 11, fgColor);
  }
}

void drawWeather(Watchy &watchy) {
  const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);

  weatherData currentWeather = watchy.getWeatherData();

  int8_t temperature = currentWeather.temperature;
  int16_t weatherConditionCode = currentWeather.weatherConditionCode;

  UiSDK::setFont(watchy.display, &monofonto10pt7b);
  UiSDK::setTextColor(watchy.display, fgColor);
  UiSDK::setCursor(watchy.display, 12, 133);

  UiSDK::print(watchy.display, temperature);
  UiSDK::print(watchy.display, currentWeather.isMetric ? "C" : "F");
  const unsigned char *weatherIcon;

  if (weatherConditionCode > 801) {  // Cloudy
    weatherIcon = cloudy;
  } else if (weatherConditionCode == 801) {  // Few Clouds
    weatherIcon = cloudsun;
  } else if (weatherConditionCode == 800) {  // Clear
    weatherIcon = sunny;
  } else if (weatherConditionCode >= 700) {  // Atmosphere
    weatherIcon = atmosphere;
  } else if (weatherConditionCode >= 600) {  // Snow
    weatherIcon = snow;
  } else if (weatherConditionCode >= 500) {  // Rain
    weatherIcon = rain;
  } else if (weatherConditionCode >= 300) {  // Drizzle
    weatherIcon = drizzle;
  } else if (weatherConditionCode >= 200) {  // Thunderstorm
    weatherIcon = thunderstorm;
  } else
    return;
  UiSDK::drawBitmap(watchy.display, 5, 85, weatherIcon, WEATHER_ICON_WIDTH, WEATHER_ICON_HEIGHT, fgColor);
}

}  // namespace

void showWatchFace_PipBoy(Watchy &watchy) {
  UiSDK::initScreen(watchy.display);
  const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);
  const uint16_t bgColor = UiSDK::getWatchfaceBg(BASE_POLARITY);

  UiSDK::setFont(watchy.display, &monofonto8pt7b);
  UiSDK::setTextColor(watchy.display, fgColor);
  UiSDK::setCursor(watchy.display, 22, 14);
  UiSDK::print(watchy.display, "STAT  INV  DATA  MAP");
  UiSDK::drawBitmap(watchy.display, 0, 10, menubar, 200, 9, fgColor);

  UiSDK::setFont(watchy.display, &monofonto8pt7b);
  UiSDK::setCursor(watchy.display, 10, 195);
  UiSDK::println(watchy.display, "PIP-BOY 3000 ROBCO IND.");

  (void)bgColor;
  drawTime(watchy);
  drawDate(watchy);
  drawSteps(watchy);
  drawWeather(watchy);
  drawBattery(watchy);
}
