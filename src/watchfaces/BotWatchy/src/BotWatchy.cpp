#include "BotWatchy.h"
#include "../../../sdk/UiSDK.h"

#ifndef WEATHER_UPDATE_INTERVAL
#define WEATHER_UPDATE_INTERVAL 30
#endif

const int posHeart0X = 10;
const int posHeart0Y = 10;
const int posHeart1X = 40;
const int posHeart1Y = 10;
const int posHeart2X = 70;
const int posHeart2Y = 10;
const int posTemperatureX = 144;
const int posTemperatureY = 93;
const int posTriforceX = 9;
const int posTriforceY = 162;
const int posWeatherBaseX = 44;
const int posWeatherBaseY = 150;
const int posWeather0X = 59;
const int posWeather0Y = 157;
const int posWeather1X = 101;
const int posWeather1Y = 157;
const int posWeather2X = 144;
const int posWeather2Y = 157;
const int posWifiX = 144;
const int posWifiY = 37;

const float VOLTAGE_MIN = 3.2;
const float VOLTAGE_MAX = 4.1;
const float VOLTAGE_RANGE = 0.9;

RTC_DATA_ATTR int weatherIntervalCounterOneCall = WEATHER_UPDATE_INTERVAL;
RTC_DATA_ATTR weatherDataOneCall currentWeatherOneCall;

namespace {

void drawTime(Watchy &watchy) {
  UiSDK::setFont(watchy.display, &Calamity_Bold18pt7b);
  UiSDK::setCursor(watchy.display, 12, 140);
  if (watchy.currentTime.Hour < 10)
    UiSDK::print(watchy.display, "0");
  UiSDK::print(watchy.display, watchy.currentTime.Hour);
  UiSDK::print(watchy.display, ":");
  if (watchy.currentTime.Minute < 10)
    UiSDK::print(watchy.display, "0");
  UiSDK::println(watchy.display, watchy.currentTime.Minute);
}

void drawDate(Watchy &watchy) {
  UiSDK::setFont(watchy.display, &Calamity_Bold8pt7b);

  String dayOfWeek = dayStr(watchy.currentTime.Wday);
  String month = monthStr(watchy.currentTime.Month);

  UiSDK::setCursor(watchy.display, 12, 68);
  UiSDK::println(watchy.display, dayOfWeek);

  UiSDK::setCursor(watchy.display, 12, 87);
  UiSDK::print(watchy.display, month);
  UiSDK::print(watchy.display, " ");

  UiSDK::print(watchy.display, watchy.currentTime.Day);
  if (watchy.currentTime.Day == 1)
    UiSDK::print(watchy.display, "st");
  else if (watchy.currentTime.Day == 2)
    UiSDK::print(watchy.display, "nd");
  else if (watchy.currentTime.Day == 3)
    UiSDK::print(watchy.display, "rd");
  else
    UiSDK::print(watchy.display, "th");
}

void drawBattery(Watchy &watchy) {
  const uint16_t fg = UiSDK::getWatchfaceFg(BASE_POLARITY);
  float VBAT = watchy.getBatteryVoltage();

  int batState = int(((VBAT - VOLTAGE_MIN) / VOLTAGE_RANGE) * 12);
  if (batState > 12)
    batState = 12;
  if (batState < 0)
    batState = 0;

  if (batState == 12) {
    UiSDK::drawBitmap(watchy.display, posHeart0X, posHeart0Y, epd_bitmap_heart_full, 27, 22, fg);
    UiSDK::drawBitmap(watchy.display, posHeart1X, posHeart1Y, epd_bitmap_heart_full, 27, 22, fg);
    UiSDK::drawBitmap(watchy.display, posHeart2X, posHeart2Y, epd_bitmap_heart_full, 27, 22, fg);
  } else if (batState == 11) {
    UiSDK::drawBitmap(watchy.display, posHeart0X, posHeart0Y, epd_bitmap_heart_full, 27, 22, fg);
    UiSDK::drawBitmap(watchy.display, posHeart1X, posHeart1Y, epd_bitmap_heart_full, 27, 22, fg);
    UiSDK::drawBitmap(watchy.display, posHeart2X, posHeart2Y, epd_bitmap_heart_threequarters, 27, 22, fg);
  } else if (batState == 10) {
    UiSDK::drawBitmap(watchy.display, posHeart0X, posHeart0Y, epd_bitmap_heart_full, 27, 22, fg);
    UiSDK::drawBitmap(watchy.display, posHeart1X, posHeart1Y, epd_bitmap_heart_full, 27, 22, fg);
    UiSDK::drawBitmap(watchy.display, posHeart2X, posHeart2Y, epd_bitmap_heart_half, 27, 22, fg);
  } else if (batState == 9) {
    UiSDK::drawBitmap(watchy.display, posHeart0X, posHeart0Y, epd_bitmap_heart_full, 27, 22, fg);
    UiSDK::drawBitmap(watchy.display, posHeart1X, posHeart1Y, epd_bitmap_heart_full, 27, 22, fg);
    UiSDK::drawBitmap(watchy.display, posHeart2X, posHeart2Y, epd_bitmap_heart_quarter, 27, 22, fg);
  } else if (batState == 8) {
    UiSDK::drawBitmap(watchy.display, posHeart0X, posHeart0Y, epd_bitmap_heart_full, 27, 22, fg);
    UiSDK::drawBitmap(watchy.display, posHeart1X, posHeart1Y, epd_bitmap_heart_full, 27, 22, fg);
    UiSDK::drawBitmap(watchy.display, posHeart2X, posHeart2Y, epd_bitmap_heart_empty, 27, 22, fg);
  } else if (batState == 7) {
    UiSDK::drawBitmap(watchy.display, posHeart0X, posHeart0Y, epd_bitmap_heart_full, 27, 22, fg);
    UiSDK::drawBitmap(watchy.display, posHeart1X, posHeart1Y, epd_bitmap_heart_threequarters, 27, 22, fg);
    UiSDK::drawBitmap(watchy.display, posHeart2X, posHeart2Y, epd_bitmap_heart_empty, 27, 22, fg);
  } else if (batState == 6) {
    UiSDK::drawBitmap(watchy.display, posHeart0X, posHeart0Y, epd_bitmap_heart_full, 27, 22, fg);
    UiSDK::drawBitmap(watchy.display, posHeart1X, posHeart1Y, epd_bitmap_heart_half, 27, 22, fg);
    UiSDK::drawBitmap(watchy.display, posHeart2X, posHeart2Y, epd_bitmap_heart_empty, 27, 22, fg);
  } else if (batState == 5) {
    UiSDK::drawBitmap(watchy.display, posHeart0X, posHeart0Y, epd_bitmap_heart_full, 27, 22, fg);
    UiSDK::drawBitmap(watchy.display, posHeart1X, posHeart1Y, epd_bitmap_heart_quarter, 27, 22, fg);
    UiSDK::drawBitmap(watchy.display, posHeart2X, posHeart2Y, epd_bitmap_heart_empty, 27, 22, fg);
  } else if (batState == 4) {
    UiSDK::drawBitmap(watchy.display, posHeart0X, posHeart0Y, epd_bitmap_heart_full, 27, 22, fg);
    UiSDK::drawBitmap(watchy.display, posHeart1X, posHeart1Y, epd_bitmap_heart_empty, 27, 22, fg);
    UiSDK::drawBitmap(watchy.display, posHeart2X, posHeart2Y, epd_bitmap_heart_empty, 27, 22, fg);
  } else if (batState == 3) {
    UiSDK::drawBitmap(watchy.display, posHeart0X, posHeart0Y, epd_bitmap_heart_threequarters, 27, 22, fg);
    UiSDK::drawBitmap(watchy.display, posHeart1X, posHeart1Y, epd_bitmap_heart_empty, 27, 22, fg);
    UiSDK::drawBitmap(watchy.display, posHeart2X, posHeart2Y, epd_bitmap_heart_empty, 27, 22, fg);
  } else if (batState == 2) {
    UiSDK::drawBitmap(watchy.display, posHeart0X, posHeart0Y, epd_bitmap_heart_half, 27, 22, fg);
    UiSDK::drawBitmap(watchy.display, posHeart1X, posHeart1Y, epd_bitmap_heart_empty, 27, 22, fg);
    UiSDK::drawBitmap(watchy.display, posHeart2X, posHeart2Y, epd_bitmap_heart_empty, 27, 22, fg);
  } else if (batState == 1) {
    UiSDK::drawBitmap(watchy.display, posHeart0X, posHeart0Y, epd_bitmap_heart_quarter, 27, 22, fg);
    UiSDK::drawBitmap(watchy.display, posHeart1X, posHeart1Y, epd_bitmap_heart_empty, 27, 22, fg);
    UiSDK::drawBitmap(watchy.display, posHeart2X, posHeart2Y, epd_bitmap_heart_empty, 27, 22, fg);
  } else if (batState == 0) {
    UiSDK::drawBitmap(watchy.display, posHeart0X, posHeart0Y, epd_bitmap_heart_empty, 27, 22, fg);
    UiSDK::drawBitmap(watchy.display, posHeart1X, posHeart1Y, epd_bitmap_heart_empty, 27, 22, fg);
    UiSDK::drawBitmap(watchy.display, posHeart2X, posHeart2Y, epd_bitmap_heart_empty, 27, 22, fg);
  }
}

void drawWeatherIcon(Watchy &watchy, int8_t iconPosX, int16_t iconWeatherConditionCode) {
  const uint16_t bg = UiSDK::getWatchfaceBg(BASE_POLARITY);
  int iconPosPxX = 0;
  if (iconPosX == 0)
    iconPosPxX = posWeather0X;
  if (iconPosX == 1)
    iconPosPxX = posWeather1X;
  if (iconPosX == 2)
    iconPosPxX = posWeather2X;

  if (iconWeatherConditionCode > 801)
    UiSDK::drawBitmap(watchy.display, iconPosPxX, posWeather0Y, epd_bitmap_weather_clouds, 27, 27, bg);
  else if (iconWeatherConditionCode == 801)
    UiSDK::drawBitmap(watchy.display, iconPosPxX, posWeather0Y, epd_bitmap_weather_partlycloudy, 27, 27, bg);
  else if (iconWeatherConditionCode == 800)
    UiSDK::drawBitmap(watchy.display, iconPosPxX, posWeather0Y, epd_bitmap_weather_sun, 27, 27, bg);
  else if (iconWeatherConditionCode >= 700)
    UiSDK::drawBitmap(watchy.display, iconPosPxX, posWeather0Y, epd_bitmap_weather_clouds, 27, 27, bg);
  else if (iconWeatherConditionCode >= 600)
    UiSDK::drawBitmap(watchy.display, iconPosPxX, posWeather0Y, epd_bitmap_weather_snow, 27, 27, bg);
  else if (iconWeatherConditionCode >= 500)
    UiSDK::drawBitmap(watchy.display, iconPosPxX, posWeather0Y, epd_bitmap_weather_rain, 27, 27, bg);
  else if (iconWeatherConditionCode >= 300)
    UiSDK::drawBitmap(watchy.display, iconPosPxX, posWeather0Y, epd_bitmap_weather_rain, 27, 27, bg);
  else if (iconWeatherConditionCode >= 200)
    UiSDK::drawBitmap(watchy.display, iconPosPxX, posWeather0Y, epd_bitmap_weather_flash, 27, 27, bg);
}

weatherDataOneCall getWeatherData(Watchy &watchy) {
  if (weatherIntervalCounterOneCall >= WEATHER_UPDATE_INTERVAL) {
    if (watchy.connectWiFi()) {
      HTTPClient http;
      http.setConnectTimeout(3000);
      String weatherQueryURL = String("https://api.openweathermap.org/data/2.5/onecall?lat=") + String(watchy.settings.lat) + String("&lon=") + String(watchy.settings.lon) + String("&exclude=minutely,hourly,alerts&units=metric&appid=") + String(watchy.settings.weatherAPIKey);
      http.begin(weatherQueryURL.c_str());
      int httpResponseCode = http.GET();
      if (httpResponseCode == 200) {
        String payload = http.getString();
        JSONVar responseObject = JSON.parse(payload);
        currentWeatherOneCall.invalid = false;
        currentWeatherOneCall.temperature = int(responseObject["current"]["temp"]);
        currentWeatherOneCall.weatherConditionCode0 = int(responseObject["current"]["weather"][0]["id"]);
        currentWeatherOneCall.weatherConditionCode1 = int(responseObject["daily"][1]["weather"][0]["id"]);
        currentWeatherOneCall.weatherConditionCode2 = int(responseObject["daily"][2]["weather"][0]["id"]);
      } else {
        currentWeatherOneCall.invalid = true;
      }
      http.end();
      WiFi.mode(WIFI_OFF);
      btStop();
    } else {
      currentWeatherOneCall.invalid = true;
    }
    weatherIntervalCounterOneCall = 0;
  } else {
    weatherIntervalCounterOneCall++;
  }
  return currentWeatherOneCall;
}

void drawWeather(Watchy &watchy) {
  const uint16_t bg = UiSDK::getWatchfaceBg(BASE_POLARITY);
  const uint16_t fg = UiSDK::getWatchfaceFg(BASE_POLARITY);
  weatherDataOneCall currentWeatherOneCall = getWeatherData(watchy);

  UiSDK::drawBitmap(watchy.display, posWeatherBaseX, posWeatherBaseY, epd_bitmap_weather_base, 150, 40, fg);
  UiSDK::drawBitmap(watchy.display, posTemperatureX, posTemperatureY, epd_bitmap_temperature_base, 50, 50, fg);

  drawWeatherIcon(watchy, 0, currentWeatherOneCall.weatherConditionCode0);
  drawWeatherIcon(watchy, 1, currentWeatherOneCall.weatherConditionCode1);
  drawWeatherIcon(watchy, 2, currentWeatherOneCall.weatherConditionCode2);

  int temperature = currentWeatherOneCall.temperature;

  int l = 16;
  int minTemp = -12;
  int maxTemp = 32;

  int scalingForMap = 10000;
  float threeQuarterPi = 4.7123;
  int scaledThreeQuarterPi = threeQuarterPi * scalingForMap;

  int scaledAngle = map(temperature, minTemp, maxTemp, 0, scaledThreeQuarterPi);
  float angle = scaledAngle / float(scalingForMap);
  if (angle > threeQuarterPi)
    angle = threeQuarterPi;
  if (angle < 0)
    angle = 0;
  angle += 2.3561;

  int startX = posTemperatureX + 25;
  int startY = posTemperatureY + 25;
  int endX = int(cos(angle) * l) + startX;
  int endY = int(sin(angle) * l) + startY;

  UiSDK::drawLine(watchy.display, startX, startY, endX, endY, bg);
  UiSDK::drawLine(watchy.display, startX + 1, startY, endX + 1, endY, bg);
  UiSDK::drawLine(watchy.display, startX, startY + 1, endX, endY + 1, bg);
}

void drawWifi(Watchy &watchy) {
  const uint16_t fg = UiSDK::getWatchfaceFg(BASE_POLARITY);
  UiSDK::drawBitmap(watchy.display, posWifiX, posWifiY, WIFI_CONFIGURED ? epd_bitmap_wifi_on : epd_bitmap_wifi_off, 50, 50, fg);
}

} // namespace

void showWatchFace_BotWatchy(Watchy &watchy) {
  UiSDK::initScreen(watchy.display);
  const uint16_t bg = UiSDK::getWatchfaceBg(BASE_POLARITY);
  const uint16_t fg = UiSDK::getWatchfaceFg(BASE_POLARITY);

  UiSDK::fillScreen(watchy.display, bg);
  UiSDK::setTextColor(watchy.display, fg);

  UiSDK::drawBitmap(watchy.display, posTriforceX, posTriforceY, epd_bitmap_triforce, 33, 28, fg);

  drawTime(watchy);
  drawDate(watchy);
  drawWeather(watchy);
  drawBattery(watchy);
  drawWifi(watchy);
}
