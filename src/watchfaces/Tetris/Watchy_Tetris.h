#ifndef WATCHY_TETRIS_H
#define WATCHY_TETRIS_H

#include "../../watchy/Watchy.h"
#include "../../sdk/UiSDK.h"
#include "tetris.h"

#include "wta.h"

class WatchyTetris : public WatchySyncedTetris
{
    using WatchySyncedTetris::WatchySyncedTetris;
    public:
        void drawWatchFace();
        void drawNumber(int x, int y, int value, int max_digits);
        double random();

    private:
        uint16_t fgColor_ = UiSDK::getWatchfaceFg(BASE_POLARITY);
};

#endif