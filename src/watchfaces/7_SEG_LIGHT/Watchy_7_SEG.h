#ifndef WATCHY_7_SEG_LIGHT_H
#define WATCHY_7_SEG_LIGHT_H

#include "../../watchy/Watchy.h"
#include "Seven_Segment10pt7b.h"
#include "DSEG7_Classic_Regular_15.h"
#include "DSEG7_Classic_Bold_25.h"
#include "DSEG7_Classic_Regular_39.h"
#include "icons.h"

class Watchy7SEGLight : public Watchy{
    using Watchy::Watchy;
    public:
        void drawWatchFace();
};

#endif