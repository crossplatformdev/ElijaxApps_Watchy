#include "Calculator.h"

#define DARKMODE false

const uint8_t BATTERY_SEGMENT_WIDTH = 7;
const uint8_t BATTERY_SEGMENT_HEIGHT = 9;
const uint8_t BATTERY_SEGMENT_SPACING = 9;
const uint8_t BATTERYX = 173;
const uint8_t BATTERYY = 10;
const uint8_t WEATHER_ICON_WIDTH = 48;
const uint8_t WEATHER_ICON_HEIGHT = 32;
const uint8_t TEMPX = 110;
const uint8_t TEMPY = 20;
const uint8_t ROW = 90;
const uint8_t ROWGAP = 2;
const uint8_t BUTTONH = 25;
const uint8_t BUTTONW = 45;
const uint8_t TEXTOFFSET = 18;
const uint8_t TIMEX = 90;
const uint8_t TIMEY = 62;
const uint8_t DOWX = 10;
const uint8_t DOWY = 80;
const uint8_t MONTHX = 100;
const uint8_t YEARX = 155;
const uint8_t STEPSX = 3;
const uint8_t STEPSY = 2;
const char* Calculator::dayStrShort(uint8_t day)
{
 const char* dayStrShort[7] = { "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT" };
    return dayStrShort[day - 1];
}

void Calculator::drawBackground() {
    display.drawLine(0, 27, 200, 27, DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
    display.drawLine(0, 85, 200, 85, DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
    int r = ROW;
    for (int y = 0; y < 4; y = y + 1) {
        for (int i = 2; i < 200; i = i + 50) {
            display.fillRoundRect(i, r, BUTTONW, BUTTONH, 7, DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
        }
        r = r + ROWGAP + BUTTONH;
    }
 
    display.setFont(&FreeSansBold10pt7b);
    display.setTextColor(DARKMODE ? GxEPD_BLACK : GxEPD_WHITE);
    
    display.setCursor(20, ROW + TEXTOFFSET);
    display.print("7");
    display.setCursor(70, ROW + TEXTOFFSET);
    display.print("8");
    display.setCursor(120, ROW + TEXTOFFSET);
    display.print("9");
    display.setCursor(170, ROW + TEXTOFFSET);
    display.print("/");
    
    display.setCursor(20, ROW + (ROWGAP + BUTTONH) + TEXTOFFSET);
    display.print("4");
    display.setCursor(70, ROW + (ROWGAP + BUTTONH) + TEXTOFFSET);
    display.print("5");
    display.setCursor(120, ROW + (ROWGAP + BUTTONH) + TEXTOFFSET);
    display.print("6");
    display.setCursor(170, ROW + (ROWGAP + BUTTONH) + TEXTOFFSET);
    display.print("x");

    display.setCursor(20, ROW + (ROWGAP + BUTTONH) * 2 + TEXTOFFSET);
    display.print("1");
    display.setCursor(70, ROW + (ROWGAP + BUTTONH) * 2 + TEXTOFFSET);
    display.print("2");
    display.setCursor(120, ROW + (ROWGAP + BUTTONH) * 2 + TEXTOFFSET);
    display.print("3");
    display.setCursor(170, ROW + (ROWGAP + BUTTONH) * 2 + TEXTOFFSET);
    display.print("-");

    display.setCursor(20, ROW + (ROWGAP + BUTTONH) * 3 + TEXTOFFSET);
    display.print("0");
    display.setCursor(70, ROW + (ROWGAP + BUTTONH) * 3 + TEXTOFFSET);
    display.print(".");
    display.setCursor(120, ROW + (ROWGAP + BUTTONH) * 3 + TEXTOFFSET);
    display.print("=");
    display.setCursor(170, ROW + (ROWGAP + BUTTONH) * 3 + TEXTOFFSET);
    display.print("+");
}

void Calculator::drawWatchFace(){
    display.fillScreen(DARKMODE ? GxEPD_BLACK : GxEPD_WHITE);
    drawBackground();
    drawTime();
    drawDate();
    drawBattery();
    drawSteps();
    drawWeather();
    }

void Calculator::drawTime(){
    display.setFont(&DSEG7_Classic_Bold_30);
    display.setCursor(TIMEX, TIMEY);
    uint8_t displayHour;

    if(HOUR_12_24==12){
      displayHour = ((currentTime.Hour+11)%12)+1;
    } else {
      displayHour = currentTime.Hour;
    }
    if(displayHour < 10){
        display.setTextColor(DARKMODE ? GxEPD_BLACK : GxEPD_WHITE);
        display.print("0");
    }
    display.setTextColor(DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
    display.print(displayHour);
    display.print(":");
    if(currentTime.Minute < 10){
        display.print("0");
    }
    display.println(currentTime.Minute);
}

void Calculator::drawDate(){
    display.setTextColor(DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
    display.setFont(&Seven_Segment10pt7b);
    
    //Draw day of week
    String dayOfWeek = dayStrShort(currentTime.Wday);
    //String dayOfWeek = "Mon";
    display.setCursor(DOWX, DOWY);
    display.println(dayOfWeek);

    //Draw month
    display.setCursor(MONTHX, DOWY);
    if (currentTime.Month < 10) {
        display.setTextColor(DARKMODE ? GxEPD_BLACK : GxEPD_WHITE);
        display.print("0");
    }
    display.setTextColor(DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
    display.print(currentTime.Month);
    if (currentTime.Day < 10) {
        display.print("-0");
    }
    else {
        display.print("-");
    };
    display.println(currentTime.Day);

    //Draw year
    display.setCursor(YEARX, DOWY);
    display.println(currentTime.Year + 1970);
}

void Calculator::drawSteps(){
    display.setFont(&Seven_Segment10pt7b);
    // reset step counter at midnight
    if (currentTime.Hour == 0 && currentTime.Minute == 0){
      sensor.resetStepCounter();
    }
    uint32_t stepCount = sensor.getCounter();
    display.drawBitmap(STEPSX, STEPSY, steps, 19, 23, DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
    display.setCursor(STEPSX + 23, STEPSY + 18);
    display.println(stepCount);
}

void Calculator::drawBattery(){
    //display.drawRect(BATTERY_LEFT - 4, 45, 27, 14, DARKMODE ? GxEPD_WHITE : GxEPD_BLACK); //battery outline
    display.drawRect(BATTERYX - 3, BATTERYY - 3, 25, 12, DARKMODE ? GxEPD_WHITE : GxEPD_BLACK); //battery outline
    int8_t batteryLevel = 0;
    float VBAT = getBatteryVoltage();
    if(VBAT > 4.1){
        batteryLevel = 3;
    }
    else if(VBAT > 3.95 && VBAT <= 4.1){
        batteryLevel = 2;
    }
    else if(VBAT > 3.80 && VBAT <= 3.95){
        batteryLevel = 1;
    }
    else if(VBAT <= 3.80){
        batteryLevel = 0;
    }

    for(int8_t batterySegments = 0; batterySegments < batteryLevel; batterySegments++){//filled in battery segments
        display.fillRect(BATTERYX + (batterySegments * (BATTERY_SEGMENT_SPACING - 2)), BATTERYY, BATTERY_SEGMENT_WIDTH -2, BATTERY_SEGMENT_HEIGHT - 3, DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
    }
}

void Calculator::drawWeather(){

    weatherData currentWeather = getWeatherData();

    int8_t temperature = currentWeather.temperature;
    int16_t weatherConditionCode = currentWeather.weatherConditionCode;

    display.setCursor(TEMPX, TEMPY);
    display.println(temperature);
    display.setCursor(TEMPX + 24, TEMPY);
    currentWeather.isMetric ? display.print("c") : display.print("f");
    const unsigned char* weatherIcon;

    //https://openweathermap.org/weather-conditions
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
    display.drawBitmap(2, 30, weatherIcon, WEATHER_ICON_WIDTH, WEATHER_ICON_HEIGHT, DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
}

