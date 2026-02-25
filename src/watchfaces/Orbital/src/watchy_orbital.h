#ifndef WATCHY_ORBITAL_H
#define WATCHY_ORBITAL_H

#include "../../../watchy/Watchy.h"
#include <Fonts/FreeSansBold9pt7b.h>
#include <sunset.h>
#include "MoonPhases20pt7b.h"
#include "OpenWeather_Regular12pt7b.h"

#define DEG2RAD 0.0174532925

class WatchyOrbital : public Watchy{
    using Watchy::Watchy;
    public:
        SunSet sun;
        unsigned int primaryColor;
        unsigned int secondaryColor;
        bool details;
        bool twentyFour;

        WatchyOrbital(const watchySettings& s) : Watchy(s) {}
        void drawWatchFace();
        void drawWatchTime();

        void drawWatchMinute();
        void drawWatchHour();
        void drawWatchDay();
        void drawWatchMonth();

        void drawBattery();
        void drawMoon();
        void drawDayTime();
        
        void drawWeather();
        
        virtual void handleButtonPress();
    private:
        void fillArc2(float, float, unsigned int, unsigned int, unsigned int, float); //the line one, for tics
        void fillArc3(float, float, unsigned int, unsigned int, unsigned int, float); //the triangle one, for not leaving stray pixels
        unsigned int DaysPerMonth(unsigned int, unsigned int);
        void drawEllipse(int16_t x0, int16_t y0, int16_t rx, int16_t ry, uint16_t color); // https://github.com/adafruit/Adafruit-GFX-Library/pull/217
        void fillEllipse(int16_t x0, int16_t y0, int16_t rx, int16_t ry, uint16_t color); // gpt4
        void drawDetails(float angle, unsigned int radius, String value);
  };
#endif
