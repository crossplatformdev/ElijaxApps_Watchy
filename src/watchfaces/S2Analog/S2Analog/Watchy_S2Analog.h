#ifndef WATCHY_S2ANALOG_H
#define WATCHY_S2ANALOG_H

#include "../../../watchy/Watchy.h"
#include "Seven_Segment10pt7b.h"
#include "s2_numbers.h"

class WatchyS2Analog : public Watchy
{
    public:
        short colour_1;
        short colour_2;
        WatchyS2Analog();
        void handleButtonPress();
        void getCosSin(double xyPair[], double deg, unsigned int len);
        void getHand(double result[], unsigned int x, unsigned int y, unsigned int deg, unsigned int width1, unsigned int width2, unsigned int len, unsigned int tip);        
        void drawOverlay(const unsigned int data[], unsigned int osx, unsigned int osy, unsigned int colour);
        void drawTicks();
        void drawWatchFace();
        void drawTime();
        void drawDate();
        void drawSteps();
        void drawWeather();
        void drawBattery();
};

#endif
