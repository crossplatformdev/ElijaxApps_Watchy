#include <vector>
#include "Adafruit_GFX_ext.h"
#include "Images.h"
#include "FreeMonoBold7pt7b.h"
#include "OpenSans_CondBold9pt7b.h"
#include <Fonts/FreeSansBold12pt7b.h>
#include "CityWeather.h"
#include "CityWeatherService.h"

#include "../../../sdk/UiSDK.h"

CityWeather::CityWeather(const watchySettings &settings_) : Watchy(settings_), cityWeatherService(*this) {}

const uint8_t WEATHER_ICON_WIDTH = 25;
const uint8_t WEATHER_ICON_HEIGHT = 25;
extern time_t savedTime;

void CityWeather::drawStatusBar()
{
  const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);
  const uint16_t bgColor = UiSDK::getWatchfaceBg(BASE_POLARITY);

  // time
  String timeStr =
      (currentTime.Hour < 10 ? "0" : "") + String(currentTime.Hour) + ":" +
      (currentTime.Minute < 10 ? "0" : "") + String(currentTime.Minute);

  UiSDK::setTextColor(display, fgColor);
  UiSDK::setFont(display, &FreeSansBold12pt7b);
  UiSDK::setCursor(display, -1, 17);
  UiSDK::print(display, timeStr);

  UiSDK::drawBitmap(display, 136, 3, wifi, 19, 16, fgColor);
  if (!WIFI_CONFIGURED) {
    UiSDK::drawLine(display, 139, 3, 139 + 12, 3 + 14, fgColor);
    UiSDK::drawLine(display, 140, 3, 140 + 12, 3 + 14, fgColor);
  }

  // battery
  // UiSDK::drawBitmap(display, 143, 1, battery, 9, 15, GxEPD_BLACK);
  UiSDK::setTextColor(display, fgColor);
  UiSDK::setFont(display, &FreeMonoBold9pt7b);
  float voltage = getBatteryVoltage();
  int batteryPercent = constrain((voltage - 3.3) * 111.11, 0, 100);
  String batteryStr = String(batteryPercent) + "%";
  drawTextRightAligned(display, 196, 16, batteryStr);

  drawLine(display, 0, 21, 199, 21, fgColor, 3);
}

void CityWeather::drawCity()
{
  const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);

  // city & country name
  UiSDK::setFont(display, &OpenSans_CondBold9pt7b);
  String clippedCityName = clipStringToWidth (display, &OpenSans_CondBold9pt7b, locationData.city, 82);
  if (clippedCityName == "") {clippedCityName = "City name";};
  printCentered(display, clippedCityName, 153, 72);

  UiSDK::drawBitmap(display, 0, 24, city, 200, 80, fgColor);
}

void CityWeather::drawTip()
{
  const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);

    UiSDK::setFont(display, &OpenSans_CondBold9pt7b);
  printCentered(display, "To display the calendar", 100, 120, fgColor);
  printCentered(display, "and weather forecast", 100, 140, fgColor);
  printCentered(display, "you need to set up Wifi", 100, 160, fgColor);
  printCentered(display, "using the Watchy menu", 100, 180, fgColor);
  printCentered(display, "<---", 100, 200, fgColor);
}

void CityWeather::drawCalendar()
{
  const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);
  const uint16_t bgColor = UiSDK::getWatchfaceBg(BASE_POLARITY);

  DailyForecast currentWeek[7];
  cityWeatherService.getCurrentWeekForecast(currentWeek);

  for (int i = 0; i < 7; i++)
  {
    // fill current day
    tmElements_t tmNow;
    Watchy::RTC.read(tmNow);
    uint32_t today = uint32_t(tmNow.Year + 1970) * 10000 + uint32_t(tmNow.Month) * 100 + uint32_t(tmNow.Day);
    if (currentWeek[i].date == today)
    {
      fillRect(display, 1 + i*28, 105, 28, 200 - 105, fgColor, 2);
    }

    const uint16_t dayTextColor = (currentWeek[i].date == today) ? bgColor : fgColor;
    UiSDK::setTextColor(display, dayTextColor);

    // weekday
    UiSDK::setFont(display, &FreeMonoBold7pt7b);
    printCentered(display, currentWeek[i].weekDay, (i * 28) + 16, 115, dayTextColor);

    // day
    UiSDK::setFont(display, &FreeMonoBold9pt7b);
    int day = currentWeek[i].date % 100;
    printCentered(display, (String)day, (i * 28) + 14, 132, dayTextColor);
    
    // weather
    const unsigned char* weatherIcon = cityWeatherService.weatherNameFromCode(currentWeek[i].weatherCode);
    UiSDK::drawBitmap(display, i * 28 + 3, 137, weatherIcon, WEATHER_ICON_WIDTH, WEATHER_ICON_HEIGHT, dayTextColor, (dayTextColor == fgColor) ? bgColor : fgColor);

    // tMax & tMin
    UiSDK::setFont(display, &OpenSans_CondBold9pt7b);
    String tMax = currentWeek[i].tempMax > 0 ? "+" + (String)currentWeek[i].tempMax : (String)currentWeek[i].tempMax;
    String tMin = currentWeek[i].tempMin > 0 ? "+" + (String)currentWeek[i].tempMin : (String)currentWeek[i].tempMin;
    printCentered(display, tMax.c_str(), (i * 28) + 14, 178, dayTextColor);
    printCentered(display, tMin.c_str(), (i * 28) + 14, 196, dayTextColor);
    
    // lines between days
    if (i > 0)
    {
      drawLine(display, 1 + i*28, 95, 1 + i*28, 200);
    }

    if (currentWeek[i].date == today)
    {
      UiSDK::drawFastVLine(display, i * 28 + 1, 104, 200 - 104, fgColor);
      UiSDK::drawFastVLine(display, i * 28 + 29, 104, 200 - 104, fgColor);
    }
  }
  
  drawLine(display, 1, 135, 199, 135, fgColor, 2); // day bottom
}

void CityWeather::drawWatchFace()
{
  cityWeatherService.updateWifiData();  

  UiSDK::initScreen(display);

  drawStatusBar();
  drawCity();

  if (savedTime == 0 && !WIFI_CONFIGURED) {
    drawTip();
  } else {
    drawCalendar();
  }

};

void showWatchFace_CityWeather(Watchy &watchy) {
  CityWeather face(watchy.settings);
  face.currentTime = watchy.currentTime;
  face.drawWatchFace();
}