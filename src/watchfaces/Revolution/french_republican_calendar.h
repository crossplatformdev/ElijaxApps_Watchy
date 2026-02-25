#ifndef FRENCH_REPUBLICAN_CALENDAR_H
#define FRENCH_REPUBLICAN_CALENDAR_H

#include <time.h>
#include <Arduino.h>

class FrenchRepublicanCalendar
{
public:
    enum class Language
    {
        French,
        English
    };

private:
    static const unsigned int FRUCTIDOR;
    static const unsigned int REVOLUTION;
    static const char *MONTH_NAMES[];
    static const char *WEEK_DAY_NAMES[];
    static const char *YEAR_DAY_NAMES[][2];
    static const String ROMAN_NUMBER_MATRIX[][2];
    const Language lang;
    unsigned int year;
    unsigned int monthNumber;
    unsigned int dayInMonth;

public:
    FrenchRepublicanCalendar(Language lang);
    void update(const time_t &currentTime);
    unsigned int getYear() const;
    const String getRomanizedYear() const;
    const char *getMonthName() const;
    unsigned int getDay() const;
    const char *getWeekDayName() const;
    const char *getYearDayName() const;
    bool sansculottides() const;

private:
    bool isFrenchLeapYear(unsigned int year) const;
    int daysInFrenchYear(unsigned int year) const;
    int daysInFrenchMonth(unsigned int year, unsigned int month) const;
    unsigned int floorRoman(unsigned int number) const;
};

#endif
