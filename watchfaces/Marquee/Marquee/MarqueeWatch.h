#ifndef MARQUEEWATCH_H
#define MARQUEEWATCH_H

#include "../../../src/watchy/Watchy.h"

class MarqueeWatch : public Watchy {

  public:
    MarqueeWatch(const watchySettings& settings);

    void drawWatchFace() override;

  private:
    void drawBackground();
    void drawBatteryBirds();
    void drawDate();
    void drawTime();
    void drawVines();
    int getBatteryPercentage();
    int getTens(int value);
};

#endif