#include "Ghost.h"
#include "../../../sdk/UiSDK.h"

const bool MILITARY_TIME = false;

const int TIME_X_OFFSET = -33;
const int TIME_Y_OFFSET = 80; 
const int DAY_DATE_X_OFFSET = 8;
const int DAY_Y_OFFSET = -93;
const int DATE_Y_OFFSET = -73;

namespace {

int getCenteredTextRightBound(Watchy &watchy, String toCenter, int xOffset) {
    int16_t  x1, y1;
    uint16_t w, h;
    UiSDK::getTextBounds(watchy.display, toCenter, 0, 0, &x1, &y1, &w, &h);
    int rightBound = xOffset + 100-w/2;
    return rightBound;
}

void drawCentered(Watchy &watchy, String toCenter, int xOffset, int yOffset){
    int16_t  x1, y1;
    uint16_t w, h;
    UiSDK::getTextBounds(watchy.display, toCenter, 0, 0, &x1, &y1, &w, &h);
    UiSDK::setCursor(watchy.display, xOffset + 100-w/2, yOffset + 100+h/2);
    UiSDK::print(watchy.display, toCenter);  
}

String getTimeString(Watchy &watchy, bool useMilitary) {
    String timeString;
    if (useMilitary) {
        timeString = watchy.currentTime.Hour;
    }
    else if (watchy.currentTime.Hour == 0) {
        timeString = 12;
    }
    else {
        timeString = watchy.currentTime.Hour <= 12 ? watchy.currentTime.Hour : watchy.currentTime.Hour - 12;
    }
    timeString += ":";
    if (watchy.currentTime.Minute < 10){
    timeString += "0";
    }
    timeString += watchy.currentTime.Minute;
    return timeString;
}

} // namespace

void showWatchFace_Kitty(Watchy &watchy) { 
    int16_t  x1, y1;
    uint16_t w, h;
    String textToDraw;

    UiSDK::initScreen(watchy.display);
    const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);
    UiSDK::setTextColor(watchy.display, fgColor);

    UiSDK::drawBitmap(watchy.display, 0, 0, ghostImage, DISPLAY_WIDTH, DISPLAY_HEIGHT, fgColor);
    
    UiSDK::setFont(watchy.display, &Montserrat_Medium20pt7b);
    textToDraw = getTimeString(watchy, MILITARY_TIME);
    drawCentered(watchy, textToDraw, TIME_X_OFFSET, TIME_Y_OFFSET);

    if (MILITARY_TIME == false) {
        int meridiemIndicatorXOffset = getCenteredTextRightBound(watchy, textToDraw, TIME_X_OFFSET) + 10;
        UiSDK::setFont(watchy.display, &Montserrat_Medium10pt7b);
        textToDraw = watchy.currentTime.Hour <= 12 ? "AM" : "PM";
        drawCentered(watchy, textToDraw, meridiemIndicatorXOffset, TIME_Y_OFFSET + 2);
    }

    UiSDK::setFont(watchy.display, &RobotoCondensed_Light7pt7b);
    textToDraw = dayStr(watchy.currentTime.Wday);
    drawCentered(watchy, textToDraw, DAY_DATE_X_OFFSET, DAY_Y_OFFSET);
    
    UiSDK::setFont(watchy.display, &RobotoCondensed_Bold13pt7b);
    textToDraw = monthShortStr(watchy.currentTime.Month);
    textToDraw += " ";
    textToDraw += watchy.currentTime.Day;
    drawCentered(watchy, textToDraw, DAY_DATE_X_OFFSET, DATE_Y_OFFSET);
}
