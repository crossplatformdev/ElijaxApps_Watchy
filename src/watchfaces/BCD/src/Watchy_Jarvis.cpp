#include "Watchy_Jarvis.h"

#include "../../../sdk/UiSDK.h"


// For more fonts look here:
// https://learn.adafruit.com/adafruit-gfx-graphics-library/using-fonts
#define FONT_LARGE       Bitwise_m19x18pt7b
#define FONT_MEDUM       Bitwise_m19x12pt7b
#define FONT_STEPS       Bitwise_m19x10pt7b
#define FONT_SMALL       Bitwise_m19x8pt7b


WatchyJarvis::WatchyJarvis(){

}


void WatchyJarvis::handleButtonPress(){
    WatchyBase::handleButtonPress();
}


void WatchyJarvis::drawWatchFace(){
    UiSDK::initScreen(display);
    const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);

    UiSDK::drawBitmap(display, 60, 60, iron_man, 80, 80, fgColor);
    drawDate();
    drawTime();
    drawSteps();
    drawBattery();
}


void WatchyJarvis::drawTime(){
    UiSDK::setFont(display, &FONT_LARGE);
    UiSDK::setCursor(display, 80, 30);
    if(currentTime.Hour < 10){
        UiSDK::print(display, "0");
    }
    UiSDK::print(display, currentTime.Hour);
    UiSDK::print(display, ":");
    if(currentTime.Minute < 10){
        UiSDK::print(display, "0");
    }
    UiSDK::println(display, currentTime.Minute);

    const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);
    UiSDK::drawLine(display, 53,1, 195, 1, fgColor);
    UiSDK::drawLine(display, 53,2, 195, 2, fgColor);
    UiSDK::drawLine(display, 55,35, 195,35, fgColor);
    UiSDK::drawLine(display, 55,36, 195,36, fgColor);
    UiSDK::drawLine(display, 195,2, 195,135, fgColor);
    UiSDK::drawLine(display, 196,2, 196,135, fgColor);
    UiSDK::drawLine(display, 180,36, 180,135, fgColor);

    UiSDK::setFont(display, &FONT_SMALL);
    UiSDK::setCursor(display, 120, 50);
    UiSDK::println(display, "time");
    UiSDK::drawLine(display, 53,42, 115,42, fgColor);
    UiSDK::drawLine(display, 150,50, 195,50, fgColor);
}


void WatchyJarvis::drawDate(){
    int16_t  x1, y1;
    uint16_t w, h;

    const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);
    drawCircle(10,20,48,fgColor,4);
    drawCircle(10,20,38,fgColor,2);

    UiSDK::setFont(display, &FONT_SMALL);
    String dayOfWeek = dayShortStr(currentTime.Wday);
    UiSDK::getTextBounds(display, dayOfWeek, 55, 195, &x1, &y1, &w, &h);
    UiSDK::setCursor(display, 5, 20);
    UiSDK::println(display, dayOfWeek);

    UiSDK::setFont(display, &FONT_MEDUM);
    UiSDK::getTextBounds(display, "00", 55, 195, &x1, &y1, &w, &h);
    UiSDK::setCursor(display, 5, 40);
    if(currentTime.Day < 10){
        UiSDK::print(display, "0");
    }
    UiSDK::println(display, currentTime.Day);
}


void WatchyJarvis::drawBattery(){
    int16_t  x1, y1;
    uint16_t w, h;

    const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);
    const uint16_t bgColor = UiSDK::getWatchfaceBg(BASE_POLARITY);

    int8_t percentage = getBattery();
    int8_t height = 60 - float(percentage)/100 * 60;
    drawCircle(20, 170, 30, fgColor, 6);
    UiSDK::fillRect(display, 0, 140, 60, height, bgColor);
    UiSDK::fillRect(display, 0, 140, 20, 30, bgColor);
    drawCircle(20, 170, 40, fgColor, 3);

    percentage = min((int8_t) 99, percentage);
    UiSDK::setFont(display, &FONT_MEDUM);
    UiSDK::setCursor(display, 5, 180);
    if(percentage < 10) {
        UiSDK::print(display, "0");
    }
    UiSDK::println(display, percentage);

    UiSDK::setFont(display, &FONT_SMALL);
    UiSDK::setCursor(display, 10, 110);
    UiSDK::println(display, "power");
    UiSDK::getTextBounds(display, "power", 10, 195, &x1, &y1, &w, &h);
    UiSDK::drawLine(display, 10-1, 112, 10+w+1, 112, fgColor);
    UiSDK::drawLine(display, 10, 112, 5, 130, fgColor);
}


void WatchyJarvis::drawSteps(){
    int16_t  x1, y1;
    uint16_t w, h;

    UiSDK::setFont(display, &FONT_STEPS);
    uint32_t steps = sensor.getCounter();
    if(steps < 1000){
        UiSDK::getTextBounds(display, String(steps), 100, 180, &x1, &y1, &w, &h);
        UiSDK::setCursor(display, 175 - w/2, 185);
        UiSDK::println(display, steps);
    }else{
        steps = round(float(steps / 1000));
        UiSDK::getTextBounds(display, String(steps), 100, 180, &x1, &y1, &w, &h);
        UiSDK::setCursor(display, 175 - w/2, 180);
        UiSDK::println(display, steps);

        UiSDK::setFont(display, &FONT_SMALL);
        UiSDK::setCursor(display, 155, 195);
        UiSDK::println(display, "thous.");
    }

    const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);
    drawCircle(185, 185, 50, fgColor, 2);
    drawCircle(185, 185, 40, fgColor, 4);

    UiSDK::setFont(display, &FONT_SMALL);
    UiSDK::setCursor(display, 80, 170);
    UiSDK::println(display, "steps");
    UiSDK::getTextBounds(display, "steps", 80, 170, &x1, &y1, &w, &h);
    UiSDK::drawLine(display, 80-1, 172, 80+w+1, 172, fgColor);
    UiSDK::drawLine(display, 80+w, 172, 135, 182, fgColor);

    UiSDK::drawLine(display, 60, 195, 135, 195, fgColor);
}


void WatchyJarvis::drawCircle(int16_t x0, int16_t y0, int16_t r,
    uint16_t color, uint8_t width){
    for(int i=0; i < width; i++){
        UiSDK::drawCircle(display, x0, y0, r-i, color);
    }
}