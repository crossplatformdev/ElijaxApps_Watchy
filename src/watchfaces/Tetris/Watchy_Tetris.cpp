#include "Watchy_Tetris.h"
#include "tetris.h"
#include "sdk/WatchyUi.h"

static const unsigned char *tetris_nums [10] = {tetris0, tetris1, tetris2, tetris3, tetris4, tetris5, tetris6, tetris7, tetris8, tetris9};

void WatchyTetris::drawWatchFace(){
    const uint16_t foreground = WatchyUi::Theme::foreground();
    display.drawBitmap(0, 0, tetrisbg, DISPLAY_WIDTH, DISPLAY_HEIGHT, foreground);

    //Hour
    display.drawBitmap(25, 20, tetris_nums[currentTime.Hour/10], 40, 60, foreground); //first digit
    display.drawBitmap(75, 20, tetris_nums[currentTime.Hour%10], 40, 60, foreground); //second digit

    //Minute
    display.drawBitmap(25, 110, tetris_nums[currentTime.Minute/10], 40, 60, foreground); //first digit
    display.drawBitmap(75, 110, tetris_nums[currentTime.Minute%10], 40, 60, foreground); //second digit
}