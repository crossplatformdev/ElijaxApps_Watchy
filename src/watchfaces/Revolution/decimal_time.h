#ifndef DECIMAL_TIME_H
#define DECIMAL_TIME_H

#include <TimeLib.h>

class DecimalTime
{
private:
    static const unsigned int MINUTE_STEPS[];
    unsigned int decimalMinutes;
    unsigned int nextAlarmWakeMinutes;
    unsigned int nextAlarmWakeSeconds;

public:
    DecimalTime();
    void update(const tmElements_t &currentTime);
    unsigned int getHours() const;
    unsigned int getMinutes() const;
    unsigned int getNextAlarmWakeMinutes() const;
    unsigned int getNextAlarmWakeSeconds() const;
};

#endif
