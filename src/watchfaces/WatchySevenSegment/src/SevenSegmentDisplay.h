#ifndef SEVENSEGMENTDISPLAY_H
#define SEVENSEGMENTDISPLAY_H

#define DEBUG_SEVENSEGMENTDISPLAY

#include <map>
#include "../../../watchy/Watchy.h"

#include "TrapezoidSegment.h"

class SevenSegmentDisplay {
    private:
        int x, y, width, height, seg_thickness = 20 /* gets overritten */, seg_spacing = 2, border = 2, lit = GxEPD_BLACK, unlit = GxEPD_WHITE;
        std::map<char, bool> segments;
        std::multimap<char, TrapezoidSegment> segment_shapes;

    public:
        SevenSegmentDisplay(int x, int y, int width, int height);
        void setSegmentState(char segment, bool state);
        bool getSegmentState(char segment);
        TrapezoidSegment getSegmentShape(char segment);
        void display(GxEPD2_BW <WatchyDisplay, WatchyDisplay::HEIGHT> *watchDisplay);
};

#endif // SEVENSEGMENTDISPLAY_H
