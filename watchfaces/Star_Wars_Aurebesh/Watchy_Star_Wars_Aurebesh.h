#ifndef WATCHY_STAY_ON_TARGET_H
#define WATCHY_STAY_ON_TARGET_H

#ifdef WATCHY_SIM
    #include "..\..\Watchy.h"
#else // WATCHY_SIM
    #include "../../src/watchy/Watchy.h"
#endif // WATCHY_SIM

#include "Aurebesh_English-Monospace10.h"
#include "Aurebesh_English-Monospace18.h"
#include "Aurebesh_English-Monospace22.h"
#include "Aurebesh_English-Monospace38.h"
#include "RogueAF20.h"
#include "icons.h"

class WatchyStarWarsAurebesh : public Watchy {
    using Watchy::Watchy;
    public:
        void drawWatchFace();
        void drawTime();
        void drawDate();
        void drawSteps();
        void drawWeather();
        void drawBattery();
        void drawWifi();
private:
        void printNumber(const char* number, int16_t x, int16_t y, int16_t max_width);
        void printVStackedln(const char* string, int16_t x, int16_t y, uint16_t spacing = 5);
};

#endif