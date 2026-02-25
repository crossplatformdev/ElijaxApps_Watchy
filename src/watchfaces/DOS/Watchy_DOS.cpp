#include "Watchy_DOS.h"

#include "../../sdk/UiSDK.h"

void WatchyDOS::drawWatchFace(){
    const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);
    UiSDK::initScreen(display);
    char time[6];
    time[0] = '0' + ((currentTime.Hour/10)%10);
    time[1] = '0' + (currentTime.Hour%10);
    time[2] = ':';
    time[3] = '0' + ((currentTime.Minute/10)%10);
    time[4] = '0' + (currentTime.Minute%10);
    time[5] = 0;
    UiSDK::setTextColor(display, fgColor);
    UiSDK::setFont(display, &Px437_IBM_BIOS5pt7b);
    UiSDK::setCursor(display, 0, 24);
    UiSDK::println(display, "WATCHY-DOS 1.1.8");
    UiSDK::println(display, "Copyright (c) 2020");
    UiSDK::println(display, " ");
    UiSDK::print(display, "AUTOEXEC BAT ");
    UiSDK::println(display, time);
    UiSDK::print(display, "COMMAND  COM ");
    UiSDK::println(display, time);
    UiSDK::print(display, "CONFIG   SYS ");
    UiSDK::println(display, time);
    UiSDK::print(display, "ESPTOOL  PY  ");
    UiSDK::println(display, time);
    UiSDK::println(display, " ");
    UiSDK::println(display, "  4 files 563 bytes");
    UiSDK::println(display, "  2048 bytes free");
    UiSDK::println(display, " ");
    UiSDK::println(display, "<C:\\>esptool");
}

void showWatchFace_DOS(Watchy &watchy) {
    WatchyDOS face(watchy.settings);
    face.currentTime = watchy.currentTime;
    face.drawWatchFace();
}