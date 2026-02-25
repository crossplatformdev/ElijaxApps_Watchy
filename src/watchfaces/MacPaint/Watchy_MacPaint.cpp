#include "Watchy_MacPaint.h"
#include "macpaint.h"
#include "sdk/WatchyUi.h"

static const unsigned char *numbers [10] = {numbers0, numbers1, numbers2, numbers3, numbers4, numbers5, numbers6, numbers7, numbers8, numbers9};

void WatchyMacPaint::drawWatchFace(){
    const uint16_t foreground = WatchyUi::Theme::foreground();
    display.drawBitmap(0, 0, window, DISPLAY_WIDTH, DISPLAY_HEIGHT, foreground);

    //Hour
    display.drawBitmap(35, 70, numbers[currentTime.Hour/10], 38, 50, foreground); //first digit
    display.drawBitmap(70, 70, numbers[currentTime.Hour%10], 38, 50, foreground); //second digit

    //Colon
    display.drawBitmap(100, 80, colon, 11, 31, foreground); //second digit

    //Minute
    display.drawBitmap(115, 70, numbers[currentTime.Minute/10], 38, 50, foreground); //first digit
    display.drawBitmap(153, 70, numbers[currentTime.Minute%10], 38, 50, foreground); //second digit
}