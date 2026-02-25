#include "../../../watchy/Watchy.h"


class BinaryBlocks_Watchface : public Watchy{
    public:
        unsigned int primaryColor;
        unsigned int secondaryColor;
        bool hands;
        
        using Watchy::Watchy;
        void drawWatchFace();
        
        void morseTime();
        void vibMorseString(String s);
        void vibMorseChar(char c);
        virtual void handleButtonPress();
};

