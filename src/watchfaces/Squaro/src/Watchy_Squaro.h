#include "../../../watchy/Watchy.h"

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

