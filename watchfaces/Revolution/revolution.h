#ifndef REVOLUTION_H
#define REVOLUTION_H

#include "french_republican_calendar.h"
#include "decimal_time.h"
#include "../../src/watchy/Watchy.h"

class Revolution : public Watchy
{
private:
    FrenchRepublicanCalendar calendar;
    DecimalTime decimalTime;
    const bool darkMode;
    const bool yearRoman;
    const float_t handWidth;

public:
    enum class Mode
    {
        DigitalDate,
        Digital,
        AnalogDate,
        Analog
    };

    Revolution(bool darkMode, bool yearRoman, float_t handWidth, FrenchRepublicanCalendar::Language dayNameLang);
    void drawWatchFace() override;
    // Reimplemented from Watchy to use ALARM1 instead of ALARM2
    void init(String datetime = "");
    // Reimplemented from Watchy to use ALARM1 instead of ALARM2 and to switch between modes
    void handleButtonPress();

private:
    void drawDigitalTime();
    void drawAnalogTime();
    void drawDate();
    void drawCenteredString(const char *str, const int x, const int y);
    void resetAlarm();
    // Reimplemented from Watchy to use ALARM1 instead of ALARM2
    void _rtcConfig(String datetime);
};

#endif
