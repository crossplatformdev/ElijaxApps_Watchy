#include "Watchy_Star_Wars_Aurebesh.h"

#include "../../sdk/UiSDK.h"

#ifndef RTC_DATA_ATTR // Define for use in Watchysim
#define RTC_DATA_ATTR
#endif

RTC_DATA_ATTR int8_t lastTemp = INT8_MIN;

const uint8_t BATTERY_SEGMENT_WIDTH = 7;
const uint8_t BATTERY_SEGMENT_HEIGHT = 11;
const uint8_t BATTERY_SEGMENT_SPACING = 9;
const uint8_t WEATHER_ICON_WIDTH = 48;
const uint8_t WEATHER_ICON_HEIGHT = 32;

void WatchyStarWarsAurebesh::drawWatchFace(){
    UiSDK::initScreen(display);
    const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);
    const uint16_t bgColor = UiSDK::getWatchfaceBg(BASE_POLARITY);

    UiSDK::setTextColor(display, fgColor);
    
    UiSDK::fillRect(display, 2, 80, 196, 2, fgColor);
    UiSDK::fillRect(display, 2, 129, 196, 2, fgColor);

    drawTime();
    drawDate();
    //drawSteps();
    drawWeather();
    drawBattery();
    UiSDK::drawBitmap(display, 18, 90, rebellogo, 30, 30, fgColor);
    drawWifi();
    if(BLE_CONFIGURED){
     //   UiSDK::drawBitmap(display, 100, 75, bluetooth, 13, 21, DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
    }
}

void WatchyStarWarsAurebesh::drawTime(){
    uint8_t displayHour;
    if(HOUR_12_24==12){
      displayHour = ((currentTime.Hour+11)%12)+1;
    } else {
      displayHour = currentTime.Hour;
    }

    UiSDK::setFont(display, &Aurebesh_English_Monospace38pt7b);
    UiSDK::setCursor(display, 18, 60);

    if (currentTime.Hour < 10) {
        UiSDK::print(display, "0");
    }
    UiSDK::println(display, currentTime.Hour);

    UiSDK::setFont(display, &Aurebesh_English_Monospace22pt7b);
    UiSDK::setCursor(display, 122, 42);

    if(currentTime.Minute < 10){
        UiSDK::print(display, "0");
    }
    UiSDK::println(display, currentTime.Minute);
}

void WatchyStarWarsAurebesh::printNumber(const char *number, int16_t x, int16_t y, int16_t max_width) {
    int16_t  x1, y1;
    uint16_t w, h;

    UiSDK::getTextBounds(display, number, 5, 100, &x1, &y1, &w, &h);
    UiSDK::setCursor(display, x + max_width - w, y);
    UiSDK::println(display, number);
}

void WatchyStarWarsAurebesh::printVStackedln(const char *string, int16_t x, int16_t y, uint16_t spacing) {
    String s = String(string);
    uint16_t total_height = 0;
    int16_t max_width = 0;
    int16_t  x1, y1;
    uint16_t w, h;

    for (unsigned i = 0; i < s.length(); i++) {
        String letter = s.substring(i, i + 1);

        UiSDK::getTextBounds(display, letter, 5, 100, &x1, &y1, &w, &h);
        if (w > max_width) {
            max_width = w;
        }
    }
     
    for (unsigned i = 0; i < s.length(); i++) {
        String letter = s.substring(i, i + 1);

        UiSDK::getTextBounds(display, letter, 5, 100, &x1, &y1, &w, &h);
        UiSDK::setCursor(display, x + (max_width - w) / 2, y + total_height);
        UiSDK::println(display, letter);
        total_height += h + spacing;
    }
}

void WatchyStarWarsAurebesh::drawDate(){
    int16_t  x1, y1, offset = 50; // 50 centred
    uint16_t w, h;

    UiSDK::setFont(display, &Aurebesh_English_Monospace10pt7b);
    printVStackedln(dayShortStr(currentTime.Wday), offset, 155);
    printVStackedln(monthShortStr(currentTime.Month), offset + 90, 155);

    String date;
    if (currentTime.Day < 10) {
        date = "0";
    }
    date.concat(currentTime.Day);

    UiSDK::setFont(display, &Aurebesh_English_Monospace22pt7b);
    UiSDK::getTextBounds(display, date, 5, 100, &x1, &y1, &w, &h);
    UiSDK::setCursor(display, offset + 50 - w / 2, 169);
    UiSDK::println(display, date);

    String year = String(tmYearToCalendar(currentTime.Year));
    
    UiSDK::setFont(display, &Aurebesh_English_Monospace10pt7b);
    UiSDK::getTextBounds(display, year, 5, 100, &x1, &y1, &w, &h);
    UiSDK::setCursor(display, offset + 50 - w / 2, 190);
    UiSDK::println(display, tmYearToCalendar(currentTime.Year));// offset from 1970, since year is stored in uint8_t
}

void WatchyStarWarsAurebesh::drawSteps(){
    // Step counter managed by core Watchy
    uint32_t stepCount = sensor.getCounter();
    const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);
    UiSDK::drawBitmap(display, 10, 165, steps, 19, 23, fgColor);
    UiSDK::setCursor(display, 35, 190);
    UiSDK::println(display, stepCount);
}

void WatchyStarWarsAurebesh::drawWifi() {
    UiSDK::setFont(display, &RogueAF220pt7b);
    UiSDK::setCursor(display, 160, 179);
    String wifi = WIFI_CONFIGURED ? String('v') : String('u');
    UiSDK::println(display, wifi);
}


void WatchyStarWarsAurebesh::drawBattery(){
    UiSDK::setFont(display, &RogueAF220pt7b);
    UiSDK::setCursor(display, 5, 179);

    String batteryLevel;
    float VBAT = getBatteryVoltage();
    if (VBAT > 4.1) {
        batteryLevel = String('z');
    }
    else if (VBAT > 3.95 && VBAT <= 4.1) {
        batteryLevel = String('y');
    }
    else if (VBAT > 3.80 && VBAT <= 3.95) {
        batteryLevel = String('x');
    }
    else if (VBAT <= 3.80) {
        batteryLevel = String('u');
    }
    UiSDK::println(display, batteryLevel);
}

void WatchyStarWarsAurebesh::drawWeather(){

    int16_t  x1, y1, y_offset = 115;
    uint16_t w, h;

    weatherData currentWeather = getWeatherData();
    if (currentWeather.external) {
        lastTemp = currentWeather.temperature;
    }

    UiSDK::setFont(display, &Aurebesh_English_Monospace18pt7b);
    if (lastTemp == INT8_MIN) {
        UiSDK::getTextBounds(display, "--", 0, 0, &x1, &y1, &w, &h);
        UiSDK::setCursor(display, 165 - w - 20, y_offset);
        UiSDK::println(display, "--");
    }
    else {
        UiSDK::getTextBounds(display, String(lastTemp), 0, 0, &x1, &y1, &w, &h);
        UiSDK::setCursor(display, 165 - w - 20, y_offset);
        UiSDK::println(display, String(lastTemp));
    }

    UiSDK::setFont(display, &Aurebesh_English_Monospace18pt7b);
    UiSDK::setCursor(display, 161, y_offset);
    UiSDK::println(display, currentWeather.isMetric ? "C" : "F");

    const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);
    UiSDK::drawRoundRect(display, 152, y_offset - 25, 7, 7, 1, fgColor);
    UiSDK::drawRoundRect(display, 151, y_offset - 26, 9, 9, 1, fgColor);

/*
    int16_t weatherConditionCode = currentWeather.weatherConditionCode;

    const unsigned char* weatherIcon;

    // https://openweathermap.org/weather-conditions
    if(weatherConditionCode > 801){//Cloudy
    weatherIcon = cloudy;
    }else if(weatherConditionCode == 801){//Few Clouds
    weatherIcon = cloudsun;
    }else if(weatherConditionCode == 800){//Clear
    weatherIcon = sunny;
    }else if(weatherConditionCode >=700){//Atmosphere
    weatherIcon = atmosphere;
    }else if(weatherConditionCode >=600){//Snow
    weatherIcon = snow;
    }else if(weatherConditionCode >=500){//Rain
    weatherIcon = rain;
    }else if(weatherConditionCode >=300){//Drizzle
    weatherIcon = drizzle;
    }else if(weatherConditionCode >=200){//Thunderstorm
    weatherIcon = thunderstorm;
    }else
    return;
    UiSDK::drawBitmap(display, 145, 158, weatherIcon, WEATHER_ICON_WIDTH, WEATHER_ICON_HEIGHT, DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
*/
}

void showWatchFace_Star_Wars_Aurebesh(Watchy &watchy) {
    WatchyStarWarsAurebesh face(watchy.settings);
    face.currentTime = watchy.currentTime;
    face.drawWatchFace();
}
