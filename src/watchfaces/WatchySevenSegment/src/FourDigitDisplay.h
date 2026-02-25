#ifndef FOURDIGITDISPLAY_H
#define FOURDIGITDISPLAY_H

#undef             DEBUG_FOURDIGITDISPLAY

#include "../../../watchy/Watchy.h"
#include "SevenSegmentDigit.h"

#define COLON_SCALE 10  
#define COLON_RADIUS_SCALE 4

class FourDigitDisplay {
    private:
        int x, y, width, height, digit_width;
        bool twenty_four_hour = false;
        bool colon = true, decimal_point = false;
        int colon_width = 0, colon_radius = 0;
    
        SevenSegmentDigit digits[4]={
            SevenSegmentDigit(0,0,0,0),
            SevenSegmentDigit(0,0,0,0),
            SevenSegmentDigit(0,0,0,0),
            SevenSegmentDigit(0,0,0,0)
        };

    public:
        FourDigitDisplay(int x, int y, int width, int height);
        void clearDisplay();
        void setDigit(int digit, int number);
        void setTime(int hour, int minute);
        void setSWTime(int minute, int second);
        void setInt(int number);
        void setColon(bool state);
        void setDecimalPoint(bool state);
        void setTwentyFourHour(bool state);
        // void setDecimalPoint(int digit, bool state);
        void display(GxEPD2_BW <WatchyDisplay, WatchyDisplay::HEIGHT> *watchDisplay);
};
#endif // FOURDIGITDISPLAY_H
