#ifndef MARQUEEWATCH_H
#define MARQUEEWATCH_H

#include "../../../watchy/Watchy.h"

class MarqueeWatch : public Watchy {

  public:
    MarqueeWatch(const watchySettings& settings);

    void drawWatchFace() override;

  private:
    void drawBackground(uint16_t fgColor, uint16_t bgColor);
    void drawBatteryBirds(uint16_t bgColor);
    void drawDate(uint16_t fgColor);
    void drawTime(uint16_t fgColor);
    void drawVines(uint16_t fgColor, uint16_t bgColor);
    int getBatteryPercentage();
    int getTens(int value);
};

#endif