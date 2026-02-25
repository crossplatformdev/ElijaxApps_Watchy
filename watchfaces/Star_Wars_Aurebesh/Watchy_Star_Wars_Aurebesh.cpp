#include "Watchy_Star_Wars_Aurebesh.h"

#define DARKMODE true

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
    display.fillScreen(DARKMODE ? GxEPD_BLACK : GxEPD_WHITE);
    display.setTextColor(DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
    
    display.fillRect(2, 80, 196, 2, DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
    display.fillRect(2, 129, 196, 2, DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);

    drawTime();
    drawDate();
    //drawSteps();
    drawWeather();
    drawBattery();
    display.drawBitmap(18, 90, rebellogo, 30, 30, DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
    drawWifi();
    if(BLE_CONFIGURED){
     //   display.drawBitmap(100, 75, bluetooth, 13, 21, DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
    }
}

void WatchyStarWarsAurebesh::drawTime(){
    uint8_t displayHour;
    if(HOUR_12_24==12){
      displayHour = ((currentTime.Hour+11)%12)+1;
    } else {
      displayHour = currentTime.Hour;
    }

    display.setFont(&Aurebesh_English_Monospace38pt7b);
    display.setCursor(18, 60);

    if (currentTime.Hour < 10) {
        display.print("0");
    }
    display.println(currentTime.Hour);

    display.setFont(&Aurebesh_English_Monospace22pt7b);
    display.setCursor(122, 42);

    if(currentTime.Minute < 10){
        display.print("0");
    }
    display.println(currentTime.Minute);
}

void WatchyStarWarsAurebesh::printNumber(const char *number, int16_t x, int16_t y, int16_t max_width) {
    int16_t  x1, y1;
    uint16_t w, h;

    display.getTextBounds(number, 5, 100, &x1, &y1, &w, &h);
    display.setCursor(x + max_width - w, y);
    display.println(number);
}

void WatchyStarWarsAurebesh::printVStackedln(const char *string, int16_t x, int16_t y, uint16_t spacing) {
    String s = String(string);
    uint16_t total_height = 0;
    int16_t max_width = 0;
    int16_t  x1, y1;
    uint16_t w, h;

    for (unsigned i = 0; i < s.length(); i++) {
        String letter = s.substring(i, i + 1);

        display.getTextBounds(letter, 5, 100, &x1, &y1, &w, &h);
        if (w > max_width) {
            max_width = w;
        }
    }
     
    for (unsigned i = 0; i < s.length(); i++) {
        String letter = s.substring(i, i + 1);

        display.getTextBounds(letter, 5, 100, &x1, &y1, &w, &h);
        display.setCursor(x + (max_width - w) / 2, y + total_height);
        display.println(letter);
        total_height += h + spacing;
    }
}

void WatchyStarWarsAurebesh::drawDate(){
    int16_t  x1, y1, offset = 50; // 50 centred
    uint16_t w, h;

    display.setFont(&Aurebesh_English_Monospace10pt7b);
    printVStackedln(dayShortStr(currentTime.Wday), offset, 155);
    printVStackedln(monthShortStr(currentTime.Month), offset + 90, 155);

    String date;
    if (currentTime.Day < 10) {
        date = "0";
    }
    date.concat(currentTime.Day);

    display.setFont(&Aurebesh_English_Monospace22pt7b);
    display.getTextBounds(date, 5, 100, &x1, &y1, &w, &h);
    display.setCursor(offset + 50 - w / 2, 169);
    display.println(date);

    String year = String(tmYearToCalendar(currentTime.Year));
    
    display.setFont(&Aurebesh_English_Monospace10pt7b);
    display.getTextBounds(year, 5, 100, &x1, &y1, &w, &h);
    display.setCursor(offset + 50 - w / 2, 190);
    display.println(tmYearToCalendar(currentTime.Year));// offset from 1970, since year is stored in uint8_t
}

void WatchyStarWarsAurebesh::drawSteps(){
    // reset step counter at midnight
    if (currentTime.Hour == 0 && currentTime.Minute == 0){
      sensor.resetStepCounter();
    }
    uint32_t stepCount = sensor.getCounter();
    display.drawBitmap(10, 165, steps, 19, 23, DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
    display.setCursor(35, 190);
    display.println(stepCount);
}

void WatchyStarWarsAurebesh::drawWifi() {
    display.setFont(&RogueAF220pt7b);
    display.setCursor(160, 179);
    String wifi = WIFI_CONFIGURED ? String('v') : String('u');
    display.println(wifi);
}


void WatchyStarWarsAurebesh::drawBattery(){
    display.setFont(&RogueAF220pt7b);
    display.setCursor(5, 179);

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
    display.println(batteryLevel);
}

void WatchyStarWarsAurebesh::drawWeather(){

    int16_t  x1, y1, y_offset = 115;
    uint16_t w, h;

    weatherData currentWeather = getWeatherData();
    if (currentWeather.external) {
        lastTemp = currentWeather.temperature;
    }

    display.setFont(&Aurebesh_English_Monospace18pt7b);
    if (lastTemp == INT8_MIN) {
        display.getTextBounds("--", 0, 0, &x1, &y1, &w, &h);
        display.setCursor(165 - w - 20, y_offset);
        display.println("--");
    }
    else {
        display.getTextBounds(String(lastTemp), 0, 0, &x1, &y1, &w, &h);
        display.setCursor(165 - w - 20, y_offset);
        display.println(String(lastTemp));
    }

    display.setFont(&Aurebesh_English_Monospace18pt7b);
    display.setCursor(161, y_offset);
    display.println(currentWeather.isMetric ? "C" : "F");

    display.drawRoundRect(152, y_offset - 25, 7, 7, 1, DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
    display.drawRoundRect(151, y_offset - 26, 9, 9, 1, DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);

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
    display.drawBitmap(145, 158, weatherIcon, WEATHER_ICON_WIDTH, WEATHER_ICON_HEIGHT, DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
*/
}
