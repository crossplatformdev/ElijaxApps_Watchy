#include "Watchy_LCARS.h"

#include "../../../sdk/UiSDK.h"


// For more fonts look here:
// https://learn.adafruit.com/adafruit-gfx-graphics-library/using-fonts
//#define FONT_LARGE       Bohemian_Typewriter22pt7b
//#define FONT_MEDUM       Bohemian_Typewriter18pt7b
#define FONT_LARGE    Okuda_A5PL25pt7b
#define FONT_MEDIUM   Okuda_A5PL16pt7b
#define FONT_SMALL    Okuda_A5PL14pt7b

WatchyLCARS::WatchyLCARS(){

}


void WatchyLCARS::handleButtonPress(){
    WatchyBase::handleButtonPress();

    uint64_t wakeupBit = esp_sleep_get_ext1_wakeup_status();
    if(IS_DOUBLE_TAP){
        // NOP
    }
}


void WatchyLCARS::drawWatchFace(){
    UiSDK::initScreen(display);
    const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);
    const uint16_t bgColor = UiSDK::getWatchfaceBg(BASE_POLARITY);

    // Draw background
    UiSDK::drawBitmap(display, 0, 0, lcars_img, 200, 200, fgColor);
    UiSDK::setFont(display, &FONT_SMALL);
    UiSDK::setTextColor(display, bgColor);
    UiSDK::setCursor(display, 110, 185);
    UiSDK::println(display, "STEP");
    UiSDK::setCursor(display, 60, 93);
    UiSDK::println(display, "lcars-watchy");

    // Draw data
    drawTime();
    drawDate();
    drawSteps();
    drawBattery();
    drawAlarm();
}


void WatchyLCARS::printCentered(uint16_t x, uint16_t y, String text){
    int16_t  x1, y1;
    uint16_t w, h;

    UiSDK::getTextBounds(display, text, 40, 100, &x1, &y1, &w, &h);
    UiSDK::setCursor(display, x-w/2, y+h/2);
    UiSDK::println(display, text);
}


void WatchyLCARS::drawTime(){
    const uint16_t bgColor = UiSDK::getWatchfaceBg(BASE_POLARITY);
    UiSDK::setFont(display, &FONT_LARGE);
    UiSDK::setTextColor(display, bgColor);
    String hourStr = String(currentTime.Hour);
    hourStr = currentTime.Hour < 10 ? "0" + hourStr : hourStr;

    String minStr = String(currentTime.Minute);
    minStr = currentTime.Minute < 10 ? "0" + minStr : minStr;
    printCentered(55, 25, hourStr + ":" + minStr);
}


void WatchyLCARS::drawDate(){
    const uint16_t bgColor = UiSDK::getWatchfaceBg(BASE_POLARITY);
    UiSDK::setFont(display, &FONT_SMALL);
    UiSDK::setTextColor(display, bgColor);

    String dayOfWeek = dayShortStr(currentTime.Wday);
    String dayStr = String(currentTime.Day);
    dayStr = currentTime.Day < 10 ? "0" + dayStr : dayStr;
    UiSDK::setCursor(display, 97, 25);
    UiSDK::println(display, dayStr);
    UiSDK::setCursor(display, 97, 47);
    UiSDK::println(display, dayOfWeek);
}


void WatchyLCARS::drawBattery(){
    const uint16_t bgColor = UiSDK::getWatchfaceBg(BASE_POLARITY);
    UiSDK::setFont(display, &FONT_MEDIUM);
    UiSDK::setTextColor(display, bgColor);

    int8_t bat = getBattery();
    bat = bat >= 100 ? 99 : bat;
    String batStr = String(bat);
    batStr = bat < 10 ? "0" + batStr : batStr;
    UiSDK::setCursor(display, 35, 125);
    UiSDK::println(display, "battery: ");
    UiSDK::setCursor(display, 113, 125);
    UiSDK::println(display, batStr + "%");
}


void WatchyLCARS::drawAlarm(){
    const uint16_t bgColor = UiSDK::getWatchfaceBg(BASE_POLARITY);
    UiSDK::setFont(display, &FONT_MEDIUM);
    UiSDK::setTextColor(display, bgColor);

    UiSDK::setCursor(display, 35, 151);
    UiSDK::println(display, "alarm:");
    UiSDK::setCursor(display, 113, 151);
        if(bcd_alarm_timer < 0){
        UiSDK::println(display, "off");
    } else {
            UiSDK::println(display, "T-" + String(bcd_alarm_timer));
    }
}


// void WatchyLCARS::drawTemperature(){
//     UiSDK::setFont(display, &FONT_SMALL);
//     UiSDK::setTextColor(display, BACKGROUND_COLOR);

//     uint8_t temperature = RTC.temperature() / 4;
//     UiSDK::setCursor(display, 65, 143);
//     UiSDK::println(display, "Temp. " + String(temperature));
// }


void WatchyLCARS::drawSteps(){
    const uint16_t bgColor = UiSDK::getWatchfaceBg(BASE_POLARITY);
    UiSDK::setFont(display, &FONT_MEDIUM);
    UiSDK::setTextColor(display, bgColor);

    uint32_t steps = sensor.getCounter();
    UiSDK::setCursor(display, 35, 186);
    UiSDK::println(display, steps);
}