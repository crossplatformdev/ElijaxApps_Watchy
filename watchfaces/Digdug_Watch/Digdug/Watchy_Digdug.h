#ifndef WATCHY_DIGDUG_H
#define WATCHY_DIGDUG_H

#ifdef WATCHY_SIM
#include "..\..\Watchy.h"
#else // WATCHY_SIM
#include "../../../src/watchy/Watchy.h"
#endif // WATCHY_SIM
#include "digdug.h"

#define INDEX_SIZE 4

class WatchyDigdug : public Watchy{
    public:
        using Watchy::Watchy;
        void drawWatchFace();
        void drawBackground();
        void drawBattery();
        void draw7Seg(const int &num, int index_x, int index_y);
        void drawEnemy(const int& num, int index_x, int index_y, int seed);
        void drawRock();
        void drawFlower();
        void drawDate();
};

#endif