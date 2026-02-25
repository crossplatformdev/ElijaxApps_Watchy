#include "Watchy_Linux.h"
#include "sdk/UiSDK.h"


// For more fonts look here:
// https://learn.adafruit.com/adafruit-gfx-graphics-library/using-fonts
//#define FONT_LARGE       Bohemian_Typewriter22pt7b
//#define FONT_MEDUM       Bohemian_Typewriter18pt7b
#define FONT        UbuntuMono_R12pt7b
#define FONT_BOLD   UbuntuMono_B12pt7b

WatchyLinux::WatchyLinux(){
    bcd_dark_mode = true;
}


void WatchyLinux::handleButtonPress(){
    WatchyBase::handleButtonPress();

    uint64_t wakeupBit = esp_sleep_get_ext1_wakeup_status();
    if(IS_DOUBLE_TAP){
        // NOP
    }
}


void WatchyLinux::drawWatchFace(){
    WatchyBase::drawWatchFace();
    if(watchFaceDisabled()){
        return;
    }

    drawBitmapRotate(100, 200, dont_panic, 0, DARK_GREY);

    // Date time
    String hourStr = String(currentTime.Hour);
    hourStr = currentTime.Hour < 10 ? "0" + hourStr : hourStr;
    String minStr = String(currentTime.Minute);
    minStr = currentTime.Minute < 10 ? "0" + minStr : minStr;

    String dayOfWeek = dayShortStr(currentTime.Wday);
    String dayStr = String(currentTime.Day);
    dayStr = currentTime.Day < 10 ? "0" + dayStr : dayStr;

    // Battery
    int8_t bat = getBattery();
    bat = bat >= 100 ? 99 : bat;
    String batStr = String(bat);
    batStr = bat < 10 ? "0" + batStr : batStr;

    // Step counter
    uint32_t steps = sensor.getCounter();

    // Alarm
    String alarm_str = bcd_alarm_timer < 0 ? "N/A" : String(bcd_alarm_timer) + " min.";

    // Draw background
    UiSDK::setFont(display, &FONT_BOLD);
    UiSDK::setTextColor(display, FOREGROUND_COLOR);
    UiSDK::setCursor(display, 0, 30);
    UiSDK::println(display, " deepthought:$");

    UiSDK::setFont(display, &FONT);
    UiSDK::print(display, " [TIME] ");
    UiSDK::println(display, hourStr + ":" + minStr);
    UiSDK::print(display, " [DATE] ");
    UiSDK::println(display, dayOfWeek + " " + dayStr);
    UiSDK::print(display, " [BATT] ");
    UiSDK::println(display, batStr + " %");
    UiSDK::print(display, " [ALRM] ");
    UiSDK::println(display, alarm_str);
    UiSDK::print(display, " [STEP] ");
    UiSDK::println(display, steps);

    UiSDK::setCursor(display, 0, 180);
    UiSDK::setFont(display, &FONT_BOLD);
    UiSDK::println(display, "--==|| 42 ||==--");
    // UiSDK::fillRect(display, 135, 164, 10, 20, FOREGROUND_COLOR);
}
