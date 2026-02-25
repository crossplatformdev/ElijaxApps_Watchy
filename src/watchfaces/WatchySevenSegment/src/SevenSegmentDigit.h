#ifndef SEVENSEGMENTDIGIT_H
#define SEVENSEGMENTDIGIT_H

#undef         DEBUG_SEVENSEGMENTDIGIT

#include "../../../watchy/Watchy.h"
#include "SevenSegmentDisplay.h"

class SevenSegmentDigit {
private:
    int x, y, width, height, lit = GxEPD_BLACK, unlit=GxEPD_WHITE;
    SevenSegmentDisplay sevensegment = SevenSegmentDisplay(0, 0, 0, 0);
  
public:
    SevenSegmentDigit(int x, int y, int width, int height);
    void setNumber(int number);
    void clearDigit();
    void display(GxEPD2_BW <WatchyDisplay, WatchyDisplay::HEIGHT> *watchDisplay);
};

#endif // SEVENSEGMENTDIGIT_H
