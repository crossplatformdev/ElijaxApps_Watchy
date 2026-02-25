#include "x.h"

void X::drawWatchFace(){
    display.fillScreen(GxEPD_BLACK);
    display.setTextColor(GxEPD_WHITE);
    display.drawBitmap(45, 38, x, 110, 124, GxEPD_WHITE);
    drawTime();
    drawWDay();
    drawDate();
    drawSteps();
    drawTemperature();
    drawBattery();

    if(currentTime.Hour < 22 && currentTime.Hour > 7 && currentTime.Minute == 0){
        vibMotor(100,2);
    }

    for(uint8_t i=0; i<3; i++){
        // Reduce ghosting
        display.display(true);
    }
}

void X::drawTime(){
    int16_t  x1, y1;
    uint16_t w, h;
    String hour;
    String minutes;
    display.setFont(&UnscreenMK27pt7b);
    hour = currentTime.Hour;

    display.getTextBounds(hour, 0, 0, &x1, &y1, &w, &h);
    display.setCursor(8, 107);
    if (currentTime.Hour < 10){
      display.print("0" + hour);
    } else {
      display.print(hour);
    }
    
    minutes = currentTime.Minute;
    display.getTextBounds(minutes, 0, 0, &x1, &y1, &w, &h);
    display.setCursor(125, 107);
    if (currentTime.Minute < 10){
      display.print("0" + minutes);
    } else {
      display.print(minutes);
    }
    

}

void X::drawWDay(){
    display.setFont(&UnscreenMK8pt7b);
    display.setCursor(8, 17);
    String dayOfWeek = dayStr(currentTime.Wday);
    dayOfWeek.toUpperCase();
    display.print(String(dayOfWeek));
}

void X::drawDate(){
    display.setFont(&UnscreenMK8pt7b);
    display.setCursor(8, 30);
    String monthStr = String(currentTime.Month);
    String dayStr = String(currentTime.Day);
    monthStr = currentTime.Month < 10 ? "0" + monthStr : monthStr;
    dayStr = currentTime.Day < 10 ? "0" + dayStr : dayStr;
    String dateStr = monthStr + "." + dayStr;
    display.print(String(dateStr));
}

void X::drawSteps(){
    int16_t  x1, y1;
    uint16_t w, h;
    display.setFont(&UnscreenMK20pt7b);
    uint32_t stepCount = sensor.getCounter();
    String stepStr = String(stepCount);
    for(int i=1; i<5; i++){
        stepStr = stepCount < pow(10, i) ? "0" + stepStr : stepStr;
    }
    if(currentTime.Hour == 23 && currentTime.Minute == 59){
        sensor.resetStepCounter();
    }
    display.getTextBounds(stepStr, 0, 0, &x1, &y1, &w, &h);
    display.setCursor(100-w/2, 185);
    display.print(String(stepStr));
}

void X::drawTemperature(){
    display.setFont(&UnscreenMK8pt7b);
    display.setCursor(155, 30);
    uint8_t temperatureRTC = temperatureRTC = RTC.temperature() / 4;
    if (settings.weatherUnit == "imperial") {
      temperatureRTC = temperatureRTC * (9/5) + 32;
    }
    if(temperatureRTC < 10){
    display.print("0");
    }
    display.print(temperatureRTC);
    if (settings.weatherUnit == "imperial") {
      display.print("Fh");
    } else {
      display.print("c");
    }
}

void X::drawBattery(){
    display.setTextColor(GxEPD_WHITE);
    display.setFont(&UnscreenMK8pt7b);
    display.setCursor(187, 17);
    display.print(">");
    display.setCursor(158, 17);
    float BATTV = getBatteryVoltage();
    if(BATTV > 4.10){
        display.print("xxx");
    }
    else if(BATTV > 3.85 && BATTV <= 4.10){
        display.print("   xx");
    }
    else if(BATTV > 3.60 && BATTV <= 3.85){
        display.print("      x");
    }
}
