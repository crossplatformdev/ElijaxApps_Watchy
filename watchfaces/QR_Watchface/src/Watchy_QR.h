#include "../../../src/watchy/Watchy.h"
#include "qrcode.h"


class QR_Watchface : public Watchy{
    public:
        using Watchy::Watchy;
        void drawWatchFace();
        void morseTime();
        void vibMorseString(String s);
        void vibMorseChar(char c);
        virtual void handleButtonPress();
};

extern RTC_DATA_ATTR bool modeEZ;
