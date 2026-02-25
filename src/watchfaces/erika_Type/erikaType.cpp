#include "erikaType.h"

#include "../../sdk/UiSDK.h"

const uint8_t BATTERY_SEGMENT_WIDTH = 7;
const uint8_t BATTERY_SEGMENT_HEIGHT = 11;
const uint8_t BATTERY_SEGMENT_SPACING = 9;
const uint8_t WEATHER_ICON_WIDTH = 48;
const uint8_t WEATHER_ICON_HEIGHT = 32;

#define TOP_LINE_Y 70
#define BOTTOM_LINE_Y 130
#define ICON_X 40
#define FONT1_FOR_BOTTOM Erika_Type_B12pt7b
#define FONT1_FOR_BOTTOM_SIZE 12
#define FONT2_FOR_BOTTOM Erika_Type_B10pt7b
#define FONT2_FOR_BOTTOM_SIZE 10
#define FONT1_FOR_TOP Erika_Type_B20pt7b
#define FONT1_FOR_TOP_SIZE 32
#define FONT2_FOR_TOP Erika_Type_B10pt7b
#define FONT2_FOR_TOP_SIZE 10
#define FONT_FOR_CENTER Erika_Type_B30pt7b
#define FONT_FOR_CENTER_SIZE 30

void erikaType::drawWatchFace(){
    const uint16_t bgColor = UiSDK::getWatchfaceBg(BASE_POLARITY);
    const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);

    UiSDK::initScreen(display);
    UiSDK::setTextColor(display, fgColor);
    drawTime();
    drawDate();
    drawSteps();
    drawWeather();
    drawBattery();
    UiSDK::drawBitmap(display, 200-26-5,200-18-5, WIFI_CONFIGURED ? wifi : wifioff, 26, 18, fgColor);
    if(BLE_CONFIGURED){
        UiSDK::drawBitmap(display, ICON_X, BOTTOM_LINE_Y+10, bluetooth, 13, 21, fgColor);
    }
   
    UiSDK::drawLine(display, 0,TOP_LINE_Y,200,TOP_LINE_Y, fgColor);
    UiSDK::drawLine(display, 0,BOTTOM_LINE_Y,200,BOTTOM_LINE_Y, fgColor);
    UiSDK::drawLine(display, 100,0,100,TOP_LINE_Y, fgColor);
    UiSDK::drawLine(display, 100,BOTTOM_LINE_Y,100,200, fgColor);
}

void erikaType::drawTime(){
    UiSDK::setFont(display, &FONT_FOR_CENTER);
    UiSDK::setCursor(display, 5, 100+(FONT_FOR_CENTER_SIZE/2));
    int displayHour;
    if(HOUR_12_24==12){
      displayHour = ((currentTime.Hour+11)%12)+1;
    } else {
      displayHour = currentTime.Hour;
    }
    if(displayHour < 10){
        UiSDK::print(display, " ");
    }
    
    UiSDK::print(display, displayHour);
    UiSDK::print(display, ":");
    if(currentTime.Minute < 10){
        UiSDK::print(display, "0");
    }
    UiSDK::println(display, currentTime.Minute);
}

void erikaType::drawDate(){
    UiSDK::setFont(display, &FONT1_FOR_BOTTOM);

    int16_t  x1, y1;
    uint16_t w, h;

    String dayOfWeek = dayShortStr(currentTime.Wday);
    UiSDK::getTextBounds(display, dayOfWeek,0, 0, &x1, &y1, &w, &h);

    UiSDK::setCursor(display, 110, BOTTOM_LINE_Y+FONT1_FOR_BOTTOM_SIZE+10);
    UiSDK::println(display, dayOfWeek);

    UiSDK::setCursor(display, 110+w, BOTTOM_LINE_Y+FONT1_FOR_BOTTOM_SIZE+10);
    UiSDK::println(display, currentTime.Day);
    
    UiSDK::setFont(display, &FONT2_FOR_BOTTOM);
    String month = monthShortStr(currentTime.Month);
    UiSDK::setCursor(display, 110, BOTTOM_LINE_Y+2*FONT1_FOR_BOTTOM_SIZE+20);
    UiSDK::println(display, month);



}
void erikaType::drawSteps(){
    UiSDK::setFont(display, &FONT1_FOR_TOP);
    // Step counter managed by core Watchy
    uint32_t stepCount = sensor.getCounter();
 
    int16_t  x1, y1;
    uint16_t w, h;
    UiSDK::getTextBounds(display, String(stepCount), 0, 0, &x1, &y1, &w, &h);
    UiSDK::setCursor(display, 100-10-w, TOP_LINE_Y-8);
    UiSDK::println(display, stepCount);
    
    UiSDK::setFont(display, &FONT2_FOR_TOP);
    UiSDK::getTextBounds(display, String("Steps"),0, 0, &x1, &y1, &w, &h);
    UiSDK::setCursor(display, 100-10-w, TOP_LINE_Y-8-FONT1_FOR_TOP_SIZE-10);
    UiSDK::println(display, "Steps");
}

void erikaType::drawBattery(){
    int8_t batteryLevel = 0;
    float VBAT = getBatteryVoltage();
    
    if (VBAT >= 3.3) {
        batteryLevel = 100.0*(VBAT-3.3)/0.9;
    }
    
    int16_t  x1, y1;
    uint16_t w, h;
    String batt(String(batteryLevel)+"%"); 
    
    UiSDK::setFont(display, &FONT1_FOR_BOTTOM);
    UiSDK::getTextBounds(display, batt,0, 0, &x1, &y1, &w, &h);
    UiSDK::setCursor(display, 100-10-w, BOTTOM_LINE_Y+FONT1_FOR_BOTTOM_SIZE+10);
    UiSDK::println(display, batt);

    UiSDK::setFont(display, &FONT2_FOR_BOTTOM);
    UiSDK::getTextBounds(display, String("Batt"),0, 0, &x1, &y1, &w, &h);
    UiSDK::setCursor(display, 100-10-w, BOTTOM_LINE_Y+2*FONT1_FOR_BOTTOM_SIZE+20);
    UiSDK::println(display, "Batt");

}

void erikaType::drawWeather(){

    const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);

    weatherData currentWeather = getWeatherData();

    int8_t temperature = currentWeather.temperature;
    int16_t weatherConditionCode = currentWeather.weatherConditionCode;
     
    const unsigned char* weatherIcon = 0;

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
    }
    if (weatherIcon)
            UiSDK::drawBitmap(display, 200-WEATHER_ICON_WIDTH, 5, weatherIcon, WEATHER_ICON_WIDTH, WEATHER_ICON_HEIGHT, fgColor);

    int16_t  x1, y1;
    uint16_t w, h;

    UiSDK::setFont(display, &FONT1_FOR_TOP);
    UiSDK::getTextBounds(display, String(temperature),0, 0, &x1, &y1, &w, &h);
    UiSDK::setCursor(display, 110, TOP_LINE_Y-8);
    UiSDK::println(display, temperature);
    
    UiSDK::drawBitmap(display, 110+w+4, TOP_LINE_Y-FONT1_FOR_TOP_SIZE, currentWeather.isMetric ? celsius : fahrenheit, 26, 20, fgColor);
   
}

void showWatchFace_erika_Type(Watchy &watchy) {
    erikaType face;
    face.settings = watchy.settings;
    face.currentTime = watchy.currentTime;
    face.drawWatchFace();
}
