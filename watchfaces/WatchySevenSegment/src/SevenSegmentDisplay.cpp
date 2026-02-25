#include <Arduino.h>
#include <Wire.h>
#include <string.h>
#include <vector>
#include <map>
#include "../../../src/watchy/Watchy.h"

#include "SevenSegmentDisplay.h"
#include "TrapezoidSegment.h"

SevenSegmentDisplay::SevenSegmentDisplay(int x, int y, int width, int height) {
  #ifdef DEBUG_SEVENSEGMENTDISPLAY
  Serial.println("SevenSegmentDisplay constructor start");
  #endif
    this->x = x;
    this->y = y;
    this->width = width;
    this->height = height;

    if ( x==0, y==0, width==0, height==0) {
      // nothing to do here... just a dummy object
      #ifdef DEBUG_SEVENSEGMENTDISPLAY
      Serial.println("SevenSegmentDisplay constructor end for dummy constuctor");
      #endif
      return;
    }

    // Let's work on that thickess.... and automatically adjust based upon the size of the digit
    seg_thickness = height / 10;

    segments = {
        {'a', false}, // top
        {'b', false}, // top right
        {'c', false}, // bottom right
        {'d', false}, // bottom
        {'e', false}, // bottom left
        {'f', false}, // top left
        {'g', false}, // middle
    };

    segment_shapes = {
      {'a',TrapezoidSegment(  border+seg_spacing,
                              border,

                              width-border-seg_spacing,
                              border,

                              width-border-seg_spacing-seg_thickness,
                              border+seg_thickness,

                              border+seg_spacing+seg_thickness,
                              border+seg_thickness)},
      {'d',TrapezoidSegment(  border+seg_spacing,
                              height-border,

                              width-border-seg_spacing,
                              height-border,

                              width-border-seg_spacing-seg_thickness,
                              height-border-seg_thickness,

                              border+seg_spacing+seg_thickness,
                              height-border-seg_thickness)},
      {'b',TrapezoidSegment(  width-border,
                              border+seg_spacing,

                              width-border,
                              height/2 - seg_spacing,

                              width-border-seg_thickness,
                              height/2 -seg_spacing-seg_thickness / 2,

                              width-border-seg_thickness,
                              border+seg_spacing+seg_thickness)},
      {'c',TrapezoidSegment(  width-border,
                              height/2 + seg_spacing,

                              width-border,
                              height-border-seg_spacing,

                              width-border-seg_thickness,
                              height-border-seg_spacing-seg_thickness,

                              width-border-seg_thickness,
                              height/2+seg_spacing+seg_thickness / 2)},

                              
      {'e',TrapezoidSegment(  border,
                              height/2 + seg_spacing,

                              border,
                              height-border-seg_spacing,

                              border+seg_thickness,
                              height-border-seg_spacing-seg_thickness,

                              border+seg_thickness ,
                              height/2+seg_spacing+seg_thickness / 2)},
      
      {'f',TrapezoidSegment(  border,
                              border+seg_spacing,

                              border,
                              height/2 - seg_spacing,

                              border+seg_thickness,
                              height/2 -seg_spacing-seg_thickness / 2,

                              border+seg_thickness,
                              border+seg_spacing+seg_thickness)},
      {'g',TrapezoidSegment(  border+seg_spacing ,
                              height/2,

                              width-border-seg_spacing ,
                              height/2,

                              width-border-seg_spacing-seg_thickness,
                              height/2-seg_thickness/2,

                              border+seg_spacing+seg_thickness,
                              height/2-seg_thickness/2)},
      {'g',TrapezoidSegment(  border+seg_spacing ,
                              height/2,

                              width-border-seg_spacing,
                              height/2,

                              width-border-seg_spacing-seg_thickness,
                              height/2+seg_thickness/2,

                              border+seg_spacing+seg_thickness,
                              height/2+seg_thickness/2)}

    };
    
    #ifdef DEBUG_SEVENSEGMENTDISPLAY
    Serial.println("SevenSegmentDisplay constructor done");
    #endif
}

void SevenSegmentDisplay::setSegmentState(char segment, bool state) {
    auto it = segments.find(segment);
    if (it != segments.end()) {
        it->second = state;
    } else {
      // need to add code to handle error
    }
}

bool SevenSegmentDisplay::getSegmentState(char segment) {
    auto it = segments.find(segment);
    if (it != segments.end()) {
      return it->second;
    } else {
      return false; // segment not found
    }
}

TrapezoidSegment SevenSegmentDisplay::getSegmentShape(char segment) {
    auto it = segment_shapes.find(segment);
    if (it != segment_shapes.end()) {
      return it->second;
    } else {
      return TrapezoidSegment(0,0,0,0,0,0,0,0); // segment not found
    }
}

void SevenSegmentDisplay::display(GxEPD2_BW <WatchyDisplay, WatchyDisplay::HEIGHT> *watchDisplay) {
  for( auto const &shape_item : segment_shapes ) {
    auto &seg_key = shape_item.first;
    TrapezoidSegment shape = shape_item.second;
    // epd_draw_rect(x,y,width,height,lit,framebuffer);
    #ifdef DEBUG_SEVENSEGMENTDISPLAY
    Serial.println("Looping through segments (display): " + String(seg_key)  );
    #endif
    int intensity = unlit;
    if (this->getSegmentState(seg_key))   {
      intensity = lit;
    }
    if (shape.getX1()>0) {
      // Serial.println("Drawing segment: " + String(seg_key) );
      watchDisplay->fillTriangle(x+shape.getX1(),y+shape.getY1(),x+shape.getX2(),y+shape.getY2(),x+shape.getX3(),y+shape.getY3(), intensity);
      watchDisplay->fillTriangle(x+shape.getX3(),y+shape.getY3(),x+shape.getX4(),y+shape.getY4(),x+shape.getX1(),y+shape.getY1(), intensity);
    }
  }
}
