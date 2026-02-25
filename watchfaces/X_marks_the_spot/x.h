#ifndef X_H
#define X_H

#include "../../src/watchy/Watchy.h"
#include "fonts/UnscreenMK20pt7b.h"
#include "fonts/UnscreenMK27pt7b.h"
#include "fonts/UnscreenMK8pt7b.h"
#include "x_img.h"

class X : public Watchy{
    using Watchy::Watchy;
    public:
        X();
        void drawWatchFace();
        void drawTime();
        void drawWDay();
        void drawDate();
        void drawSteps();
        void drawTemperature();
        void drawBattery();
};

#endif
