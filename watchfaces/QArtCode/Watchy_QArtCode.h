#ifndef RAND_PICS_H
#define RAND_PICS_H

#include "../../src/watchy/Watchy.h"
#include <stdlib.h>
#include <Fonts/FreeMonoBold12pt7b.h> // oh dear too big!

#include "broken.h"

class WatchyQArtCode : public Watchy{
    public:
        WatchyQArtCode();
        void drawWatchFace();
};

#endif
