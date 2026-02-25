#include <ctime>

#include "../../watchy/Watchy.h"
#include "../../sdk/UiSDK.h"
#include "gfxfont.h"
#include "Watchy_Powershell.h"
#include "px437_IBM_BIOS5pt7b.h"

void showWatchFace_PowerShell(Watchy &watchy) {
    UiSDK::initScreen(watchy.display);
    const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);
    const uint16_t bgColor = UiSDK::getWatchfaceBg(BASE_POLARITY);

    UiSDK::drawBitmap(watchy.display, 0, 0, epd_bitmap_Watchy_PowerShell, 200, 200, fgColor);
    
    (void)bgColor;
    UiSDK::setTextColor(watchy.display, fgColor);
    UiSDK::setFont(watchy.display, &Px437_IBM_BIOS5pt7b);
    UiSDK::setCursor(watchy.display, 0, 175);
    
    struct tm currentLocalTime;
    currentLocalTime.tm_wday = watchy.currentTime.Wday - 1;
    currentLocalTime.tm_year = watchy.currentTime.Year + 1970 - 1900;
    currentLocalTime.tm_mon = watchy.currentTime.Month - 1;
    currentLocalTime.tm_mday = watchy.currentTime.Day;
    currentLocalTime.tm_hour = watchy.currentTime.Hour;
    currentLocalTime.tm_min = watchy.currentTime.Minute;
    currentLocalTime.tm_sec = watchy.currentTime.Second;
    
    char buffer[20];
    strftime(buffer, sizeof(buffer), " %a %b %d, %Y", &currentLocalTime);
    UiSDK::println(watchy.display, buffer);
    
    strftime(buffer, sizeof(buffer), " %I:%M %p", &currentLocalTime);
    UiSDK::print(watchy.display, buffer);
    
    float battery = watchy.getBatteryVoltage();
    if (battery < 3.80)
    {
        UiSDK::setCursor(watchy.display, 170, 185);
        UiSDK::print(watchy.display, "BAT");
    }
}
