#include "Watchy_BCD.h"
#include "sdk/UiSDK.h"


// For more fonts look here:
// https://learn.adafruit.com/adafruit-gfx-graphics-library/using-fonts
#define FONT    FreeMonoBold12pt7b


WatchyBCD::WatchyBCD(){

}


void WatchyBCD::handleButtonPress(){
    WatchyBase::handleButtonPress();

    uint64_t wakeupBit = esp_sleep_get_ext1_wakeup_status();
    if(IS_DOUBLE_TAP){
        bcd_dark_mode = bcd_dark_mode ? false : true;
        RTC.read(currentTime);
        showWatchFace(false);
        return;
    }
}


void WatchyBCD::drawWatchFace(){
    WatchyBase::drawWatchFace();
    if(watchFaceDisabled()){
        return;
    }

    UiSDK::drawBitmap(display, 0, 0, pcb, 200, 200, FOREGROUND_COLOR);

    UiSDK::setFont(display, &FONT);
    UiSDK::setTextColor(display, FOREGROUND_COLOR);

    UiSDK::fillRect(display, 43, 5, 50, 21, BACKGROUND_COLOR);
    UiSDK::setCursor(display, 48, 20);
    UiSDK::println(display, "10b");

    UiSDK::fillRect(display, 123, 5, 50, 21, BACKGROUND_COLOR);
    UiSDK::setCursor(display, 128, 20);
    UiSDK::println(display, "01b");

    uint32_t steps = sensor.getCounter();
    uint8_t bat = getBattery();
    bat = min((uint8_t) 99, bat);

    printBCD("h", 30, currentTime.Hour);
    printBCD("m", 50, currentTime.Minute);

    printBCD("M", 80, currentTime.Month);
    printBCD("D", 100, currentTime.Day);

    printBCD("B", 130, bat);

    uint32_t setps_10k = steps / 1000;
    uint32_t steps_100 = (steps - 1000*setps_10k) / 10;
    printBCD("S", 160, setps_10k);
    printBCD("", 180, steps_100);
}


void WatchyBCD::printBCD(String s, uint16_t y, uint16_t value){
    if(s != ""){
        UiSDK::fillRect(display, 8, y-5, 22, 22, BACKGROUND_COLOR);
        UiSDK::setCursor(display, 13, y+13);
        UiSDK::println(display, s);
    }

    printBinary(38, y, (uint8_t) value / 10, 4);
    printBinary(118, y, (uint8_t) value % 10, 4);
}

void WatchyBCD::printBinary(uint16_t x, uint16_t y, uint8_t value, uint8_t n){
    if(n <= 0){
        return;
    }

    uint8_t gap = 2;
    uint8_t size = 15;
    uint8_t x_pos = x + (n-1)*(size+gap);

    UiSDK::fillRoundRect(display, x_pos-gap, y-gap-1, size+2*gap, size+2*gap+2, 3, BACKGROUND_COLOR);
    if(value % 2 == 0){
        UiSDK::drawRoundRect(display, x_pos, y, size, size, 3, FOREGROUND_COLOR);
        UiSDK::drawRoundRect(display, x_pos+1, y+1, size-2, size-2, 3, FOREGROUND_COLOR);
        UiSDK::fillRoundRect(display, x_pos+2, y+2, size-4, size-4, 3, BACKGROUND_COLOR);
    } else {
        UiSDK::fillRoundRect(display, x_pos, y, size, size, 3, FOREGROUND_COLOR);
    }

    printBinary(x, y, value / 2, n-1);
}
