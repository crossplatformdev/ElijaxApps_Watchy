#include "../../../src/watchy/Watchy.h"


class Squarbital_Watchface : public Watchy{
    public:
        unsigned int primaryColor;
        unsigned int secondaryColor;
        bool twentyFour;
        
        using Watchy::Watchy;
        void drawWatchFace();
        void printDecor();
        void drawSquare(int x, int y, int width, float angle);
        int daysPerMonth(int year, int month);
        float getBatteryAngle();
        void morseTime();
        void vibMorseString(String s);
        void vibMorseChar(char c);
        virtual void handleButtonPress();
};

extern RTC_DATA_ATTR int mode;
