#ifndef WATCHY_7_SEG_H
#define WATCHY_7_SEG_H

#include <Watchy.h>

namespace Watchy7SEG {
        void drawWatchFace(Watchy &watch);
        void drawTime(Watchy &watch);
        void drawDate(Watchy &watch);
        void drawSteps(Watchy &watch);
        void drawWeather(Watchy &watch);
        void drawBattery(Watchy &watch);
        void drawHeartRate(Watchy &watch);
        void refreshHeartRate(Watchy &watch);
        bool updateWatchFaceData(Watchy &watch);
}

#endif