#include "../../../src/watchy/Watchy.h"

extern RTC_DATA_ATTR bool darkTheme;

class Squaro_Watchface : public Watchy{
    public:
        unsigned int primaryColor;
        unsigned int secondaryColor;
        
        using Watchy::Watchy;
        void drawWatchFace();
        void morseTime();
        void vibMorseString(String s);
        void vibMorseChar(char c);
        virtual void handleButtonPress();
};

