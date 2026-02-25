#include "../../watchy/Watchy.h"
#include "../../sdk/UiSDK.h"
#include "DIN_1451_Engschrift_Regular64pt7b.h"
#include "DIN_1451_Engschrift_Regular12pt7b.h"

void showWatchFace_Bahn(Watchy &watchy) {
  int16_t  x1, y1, lasty;
  uint16_t w, h;
  String textstring;
  UiSDK::initScreen(watchy.display);

  const uint16_t fg = UiSDK::getWatchfaceFg(BASE_POLARITY);
  const uint16_t bg = UiSDK::getWatchfaceBg(BASE_POLARITY);

  // ** UPDATE **
  // Step counter managed by core Watchy

  // ** DRAW **
  
  //drawbg
  UiSDK::fillRoundRect(watchy.display, 2,2,196,196,8,fg);
  UiSDK::fillRoundRect(watchy.display, 6,6,188,188,5,bg);
  
  UiSDK::setFont(watchy.display, &DIN_1451_Engschrift_Regular64pt7b);
  UiSDK::setTextColor(watchy.display, fg);
  UiSDK::setTextWrap(watchy.display, false);

  //draw hours
  textstring = watchy.currentTime.Hour;
  UiSDK::getTextBounds(watchy.display, textstring, 0, 0, &x1, &y1, &w, &h);
  UiSDK::setCursor(watchy.display, 183-w, 100-5);
  UiSDK::print(watchy.display, textstring);
  
  //draw minutes
  if (watchy.currentTime.Minute < 10) {
    textstring = "0";
  } else {
    textstring = "";
  }
  textstring += watchy.currentTime.Minute;
  UiSDK::getTextBounds(watchy.display, textstring, 0, 0, &x1, &y1, &w, &h);
  UiSDK::setCursor(watchy.display, 183-w, 100+3+h);
  UiSDK::print(watchy.display, textstring);

  // draw battery
  UiSDK::fillRoundRect(watchy.display, 16,16,34,12,4,fg);
  UiSDK::fillRoundRect(watchy.display, 49,20,3,4,2,fg);
  UiSDK::fillRoundRect(watchy.display, 18,18,30,8,3,bg);
  float batt = (watchy.getBatteryVoltage()-3.3)/0.9;
  if (batt > 0) {
   UiSDK::fillRoundRect(watchy.display, 20,20,26*batt,4,2,fg);
  }									

  UiSDK::setFont(watchy.display, &DIN_1451_Engschrift_Regular12pt7b);
  lasty = 200 - 16;

  //draw steps
  textstring = sensor.getCounter();
  textstring += " steps";
  UiSDK::getTextBounds(watchy.display, textstring, 0, 0, &x1, &y1, &w, &h);
  UiSDK::fillRoundRect(watchy.display, 16,lasty-h-2,w + 7,h+4,2,fg);
 UiSDK::setCursor(watchy.display, 19, lasty-3);
  UiSDK::setTextColor(watchy.display, bg);
  UiSDK::print(watchy.display, textstring);
  UiSDK::setTextColor(watchy.display, fg);
  lasty += -8-h;

  // draw year
  textstring = watchy.currentTime.Year + 1970;
  UiSDK::getTextBounds(watchy.display, textstring, 0, 0, &x1, &y1, &w, &h);
  UiSDK::setCursor(watchy.display, 16, lasty);
  UiSDK::print(watchy.display, textstring);
  lasty += -20;

  // draw date
  textstring = monthShortStr(watchy.currentTime.Month);
  textstring += " ";
  textstring += watchy.currentTime.Day;
  UiSDK::getTextBounds(watchy.display, textstring, 0, 0, &x1, &y1, &w, &h);
  UiSDK::setCursor(watchy.display, 16, lasty);
  UiSDK::print(watchy.display, textstring);
  lasty += -20;
  
  // draw day
  textstring = dayStr(watchy.currentTime.Wday);
  UiSDK::getTextBounds(watchy.display, textstring, 0, 0, &x1, &y1, &w, &h);
  UiSDK::setCursor(watchy.display, 16, lasty);
  UiSDK::print(watchy.display, textstring);
  lasty += -40;

  // weather things
  weatherData currentWeather = watchy.getWeatherData();
  int8_t temperature = currentWeather.temperature;
  int16_t weatherConditionCode = currentWeather.weatherConditionCode;

  // draw weather state
  if (weatherConditionCode >= 801) {
    textstring = "Cloudy";
  } else if (weatherConditionCode == 800) {
    textstring = "Clear";
  } else if (weatherConditionCode == 781) {
    textstring = "Tornado";
  } else if (weatherConditionCode == 771) {
    textstring = "Squall";
  } else if (weatherConditionCode == 762) {
    textstring = "Ash";
  } else if (weatherConditionCode == 761 || weatherConditionCode == 731) {
    textstring = "Dust";
  } else if (weatherConditionCode == 751) {
    textstring = "Sand";
  } else if (weatherConditionCode == 741) {
    textstring = "Fog";
  } else if (weatherConditionCode == 721) {
    textstring = "Haze";
  } else if (weatherConditionCode == 711) {
    textstring = "Smoke";
  } else if (weatherConditionCode == 701) {
    textstring = "Mist";
  } else if (weatherConditionCode >= 600) {
    textstring = "Snow";
  } else if (weatherConditionCode >= 500) {
    textstring = "Rain";
  } else if (weatherConditionCode >= 300) {
    textstring = "Drizzle";
  } else if (weatherConditionCode >= 200) {
    textstring = "Thunderstorm";
  } else {
    textstring = "";
  }
  UiSDK::getTextBounds(watchy.display, textstring, 0, 0, &x1, &y1, &w, &h);
  UiSDK::setCursor(watchy.display, 16, lasty);
  UiSDK::print(watchy.display, textstring);
  lasty += -20;

  // draw temperature
  textstring = temperature;
  textstring += (strcmp(watchy.settings.weatherUnit.c_str(), "metric") == 0) ? "C" : "F";
  UiSDK::getTextBounds(watchy.display, textstring, 0, 0, &x1, &y1, &w, &h);
  UiSDK::setCursor(watchy.display, 16, lasty);
  UiSDK::print(watchy.display, textstring);
}
