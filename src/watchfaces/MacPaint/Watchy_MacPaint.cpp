#include "../../watchy/Watchy.h"
#include "../../sdk/UiSDK.h"
#include "MacPaint.h"

static const unsigned char *numbers[10] = {numbers0, numbers1, numbers2, numbers3, numbers4, numbers5, numbers6, numbers7, numbers8, numbers9};

void showWatchFace_MacPaint(Watchy &watchy){
    UiSDK::initScreen(watchy.display);
    const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);
    UiSDK::drawBitmap(watchy.display, 0, 0, window, DISPLAY_WIDTH, DISPLAY_HEIGHT, fgColor);

    //Hour
    UiSDK::drawBitmap(watchy.display, 35, 70, numbers[watchy.currentTime.Hour/10], 38, 50, fgColor); //first digit
    UiSDK::drawBitmap(watchy.display, 70, 70, numbers[watchy.currentTime.Hour%10], 38, 50, fgColor); //second digit

    //Colon
    UiSDK::drawBitmap(watchy.display, 100, 80, colon, 11, 31, fgColor); //second digit

    //Minute
    UiSDK::drawBitmap(watchy.display, 115, 70, numbers[watchy.currentTime.Minute/10], 38, 50, fgColor); //first digit
    UiSDK::drawBitmap(watchy.display, 153, 70, numbers[watchy.currentTime.Minute%10], 38, 50, fgColor); //second digit
}