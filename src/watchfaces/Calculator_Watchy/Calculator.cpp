#include "Calculator.h"

#include "../../sdk/UiSDK.h"

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

namespace {

const char* dayStrShort(uint8_t day) {
    const char* dayStrShort[7] = { "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT" };
    return dayStrShort[day - 1];
}

void drawBackground(Watchy &watchy) {
    const uint16_t fg = UiSDK::getWatchfaceFg(BASE_POLARITY);
    const uint16_t bg = UiSDK::getWatchfaceBg(BASE_POLARITY);

    UiSDK::drawLine(watchy.display, 0, 27, 200, 27, fg);
    UiSDK::drawLine(watchy.display, 0, 85, 200, 85, fg);
    int r = ROW;
    for (int y = 0; y < 4; y = y + 1) {
        for (int i = 2; i < 200; i = i + 50) {
            UiSDK::fillRoundRect(watchy.display, i, r, BUTTONW, BUTTONH, 7, fg);
        }
        r = r + ROWGAP + BUTTONH;
    }

    UiSDK::setFont(watchy.display, &FreeSansBold10pt7b);
    UiSDK::setTextColor(watchy.display, bg);
  
    UiSDK::setCursor(watchy.display, 20, ROW + TEXTOFFSET);
    UiSDK::print(watchy.display, "7");
    UiSDK::setCursor(watchy.display, 70, ROW + TEXTOFFSET);
    UiSDK::print(watchy.display, "8");
    UiSDK::setCursor(watchy.display, 120, ROW + TEXTOFFSET);
    UiSDK::print(watchy.display, "9");
    UiSDK::setCursor(watchy.display, 170, ROW + TEXTOFFSET);
    UiSDK::print(watchy.display, "/");
  
    UiSDK::setCursor(watchy.display, 20, ROW + (ROWGAP + BUTTONH) + TEXTOFFSET);
    UiSDK::print(watchy.display, "4");
    UiSDK::setCursor(watchy.display, 70, ROW + (ROWGAP + BUTTONH) + TEXTOFFSET);
    UiSDK::print(watchy.display, "5");
    UiSDK::setCursor(watchy.display, 120, ROW + (ROWGAP + BUTTONH) + TEXTOFFSET);
    UiSDK::print(watchy.display, "6");
    UiSDK::setCursor(watchy.display, 170, ROW + (ROWGAP + BUTTONH) + TEXTOFFSET);
    UiSDK::print(watchy.display, "x");

    UiSDK::setCursor(watchy.display, 20, ROW + (ROWGAP + BUTTONH) * 2 + TEXTOFFSET);
    UiSDK::print(watchy.display, "1");
    UiSDK::setCursor(watchy.display, 70, ROW + (ROWGAP + BUTTONH) * 2 + TEXTOFFSET);
    UiSDK::print(watchy.display, "2");
    UiSDK::setCursor(watchy.display, 120, ROW + (ROWGAP + BUTTONH) * 2 + TEXTOFFSET);
    UiSDK::print(watchy.display, "3");
    UiSDK::setCursor(watchy.display, 170, ROW + (ROWGAP + BUTTONH) * 2 + TEXTOFFSET);
    UiSDK::print(watchy.display, "-");

    UiSDK::setCursor(watchy.display, 20, ROW + (ROWGAP + BUTTONH) * 3 + TEXTOFFSET);
    UiSDK::print(watchy.display, "0");
    UiSDK::setCursor(watchy.display, 70, ROW + (ROWGAP + BUTTONH) * 3 + TEXTOFFSET);
    UiSDK::print(watchy.display, ".");
    UiSDK::setCursor(watchy.display, 120, ROW + (ROWGAP + BUTTONH) * 3 + TEXTOFFSET);
    UiSDK::print(watchy.display, "=");
    UiSDK::setCursor(watchy.display, 170, ROW + (ROWGAP + BUTTONH) * 3 + TEXTOFFSET);
    UiSDK::print(watchy.display, "+");
}

void drawTime(Watchy &watchy) {
    const uint16_t fg = UiSDK::getWatchfaceFg(BASE_POLARITY);
    const uint16_t bg = UiSDK::getWatchfaceBg(BASE_POLARITY);

    UiSDK::setFont(watchy.display, &DSEG7_Classic_Bold_30);
    UiSDK::setCursor(watchy.display, TIMEX, TIMEY);
    uint8_t displayHour;

    if(HOUR_12_24==12){
        displayHour = ((watchy.currentTime.Hour+11)%12)+1;
    } else {
        displayHour = watchy.currentTime.Hour;
    }
    if(displayHour < 10){
        UiSDK::setTextColor(watchy.display, bg);
        UiSDK::print(watchy.display, "0");
    }
    UiSDK::setTextColor(watchy.display, fg);
    UiSDK::print(watchy.display, displayHour);
    UiSDK::print(watchy.display, ":");
    if(watchy.currentTime.Minute < 10){
        UiSDK::print(watchy.display, "0");
    }
    UiSDK::println(watchy.display, watchy.currentTime.Minute);
}

void drawDate(Watchy &watchy) {
    const uint16_t fg = UiSDK::getWatchfaceFg(BASE_POLARITY);
    const uint16_t bg = UiSDK::getWatchfaceBg(BASE_POLARITY);

    UiSDK::setTextColor(watchy.display, fg);
    UiSDK::setFont(watchy.display, &Seven_Segment10pt7b);
  
    String dayOfWeek = dayStrShort(watchy.currentTime.Wday);
    UiSDK::setCursor(watchy.display, DOWX, DOWY);
    UiSDK::println(watchy.display, dayOfWeek);

    UiSDK::setCursor(watchy.display, MONTHX, DOWY);
    if (watchy.currentTime.Month < 10) {
        UiSDK::setTextColor(watchy.display, bg);
        UiSDK::print(watchy.display, "0");
    }
    UiSDK::setTextColor(watchy.display, fg);
    UiSDK::print(watchy.display, watchy.currentTime.Month);
    if (watchy.currentTime.Day < 10) {
        UiSDK::print(watchy.display, "-0");
    }
    else {
        UiSDK::print(watchy.display, "-");
    };
    UiSDK::println(watchy.display, watchy.currentTime.Day);

    UiSDK::setCursor(watchy.display, YEARX, DOWY);
    UiSDK::println(watchy.display, watchy.currentTime.Year + 1970);
}

void drawSteps(Watchy &watchy) {
    const uint16_t fg = UiSDK::getWatchfaceFg(BASE_POLARITY);

    UiSDK::setFont(watchy.display, &Seven_Segment10pt7b);
    uint32_t stepCount = sensor.getCounter();
    UiSDK::setTextColor(watchy.display, fg);
    UiSDK::drawBitmap(watchy.display, STEPSX, STEPSY, steps, 19, 23, fg);
    UiSDK::setCursor(watchy.display, STEPSX + 23, STEPSY + 18);
    UiSDK::println(watchy.display, stepCount);
}

void drawBattery(Watchy &watchy) {
    const uint16_t fg = UiSDK::getWatchfaceFg(BASE_POLARITY);

    UiSDK::drawRect(watchy.display, BATTERYX - 3, BATTERYY - 3, 25, 12, fg);
    int8_t batteryLevel = 0;
    float VBAT = watchy.getBatteryVoltage();
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

    for(int8_t batterySegments = 0; batterySegments < batteryLevel; batterySegments++){
        UiSDK::fillRect(watchy.display, BATTERYX + (batterySegments * (BATTERY_SEGMENT_SPACING - 2)), BATTERYY, BATTERY_SEGMENT_WIDTH -2, BATTERY_SEGMENT_HEIGHT - 3, fg);
    }
}

void drawWeather(Watchy &watchy) {
    const uint16_t fg = UiSDK::getWatchfaceFg(BASE_POLARITY);

    weatherData currentWeather = watchy.getWeatherData();

    int8_t temperature = currentWeather.temperature;
    int16_t weatherConditionCode = currentWeather.weatherConditionCode;

    UiSDK::setTextColor(watchy.display, fg);
    UiSDK::setCursor(watchy.display, TEMPX, TEMPY);
    UiSDK::println(watchy.display, temperature);
    UiSDK::setCursor(watchy.display, TEMPX + 24, TEMPY);
    currentWeather.isMetric ? UiSDK::print(watchy.display, "c") : UiSDK::print(watchy.display, "f");
    const unsigned char* weatherIcon;

    if(weatherConditionCode > 801){
        weatherIcon = cloudy;
    }else if(weatherConditionCode == 801){
        weatherIcon = cloudsun;
    }else if(weatherConditionCode == 800){
        weatherIcon = sunny;
    }else if(weatherConditionCode >=700){
        weatherIcon = atmosphere;
    }else if(weatherConditionCode >=600){
        weatherIcon = snow;
    }else if(weatherConditionCode >=500){
        weatherIcon = rain;
    }else if(weatherConditionCode >=300){
        weatherIcon = drizzle;
    }else if(weatherConditionCode >=200){
        weatherIcon = thunderstorm;
    }else
        return;
    UiSDK::drawBitmap(watchy.display, 2, 30, weatherIcon, WEATHER_ICON_WIDTH, WEATHER_ICON_HEIGHT, fg);
}

} // namespace

void showWatchFace_Calculator_Watchy(Watchy &watchy) {
    UiSDK::initScreen(watchy.display);
    drawBackground(watchy);
    drawTime(watchy);
    drawDate(watchy);
    drawBattery(watchy);
    drawSteps(watchy);
    drawWeather(watchy);
}

