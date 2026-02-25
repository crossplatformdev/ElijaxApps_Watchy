#include <Arduino.h>
#include <Wire.h>
#include <string.h>
#include <vector>
#include <map>
#include "../../../watchy/Watchy.h"

#include "SevenSegmentDigit.h"

SevenSegmentDigit::SevenSegmentDigit(int x, int y, int width, int height) {
    #ifdef DEBUG_SEVENSEGMENTDIGIT  
    Serial.println("SevenSegmentDigit constructor start with " + String(x) + ", " + String(y) + ", " + String(width) + ", " + String(height));
    #endif
    this->x = x;
    this->y = y;
    this->width = width;
    this->height = height;
    sevensegment = SevenSegmentDisplay(x, y, width, height);
    #ifdef DEBUG_SEVENSEGMENTDIGIT
    Serial.println("SevenSegmentDigit constructor done");
    #endif
  }

  void SevenSegmentDigit::setNumber(int number) {
    if (number < 0 || number > 9) {
      return;
    }

    // Clear all segments
    for (char seg = 'a'; seg <= 'g'; ++seg) {
      sevensegment.setSegmentState(seg, false);
    }

    // Set segments for the given number
   switch (number) {
         case 0: sevensegment.setSegmentState('a', true); sevensegment.setSegmentState('b', true); sevensegment.setSegmentState('c', true); sevensegment.setSegmentState('d', true); sevensegment.setSegmentState('e', true); sevensegment.setSegmentState('f', true); break;
         case 1: sevensegment.setSegmentState('b', true); sevensegment.setSegmentState('c', true); break;
         case 2: sevensegment.setSegmentState('a', true); sevensegment.setSegmentState('b', true); sevensegment.setSegmentState('d', true); sevensegment.setSegmentState('e', true); sevensegment.setSegmentState('g', true); break;
         case 3: sevensegment.setSegmentState('a', true); sevensegment.setSegmentState('b', true); sevensegment.setSegmentState('c', true); sevensegment.setSegmentState('d', true); sevensegment.setSegmentState('g', true); break;
         case 4: sevensegment.setSegmentState('b', true); sevensegment.setSegmentState('c', true); sevensegment.setSegmentState('f', true); sevensegment.setSegmentState('g', true); break;
         case 5: sevensegment.setSegmentState('a', true); sevensegment.setSegmentState('c', true); sevensegment.setSegmentState('d', true); sevensegment.setSegmentState('f', true); sevensegment.setSegmentState('g', true); break;
         case 6: sevensegment.setSegmentState('a', true); sevensegment.setSegmentState('c', true); sevensegment.setSegmentState('d', true); sevensegment.setSegmentState('e', true); sevensegment.setSegmentState('f', true); sevensegment.setSegmentState('g', true); break;
         case 7: sevensegment.setSegmentState('a', true); sevensegment.setSegmentState('b', true); sevensegment.setSegmentState('c', true); break;
         case 8: sevensegment.setSegmentState('a', true); sevensegment.setSegmentState('b', true); sevensegment.setSegmentState('c', true); sevensegment.setSegmentState('d', true); sevensegment.setSegmentState('e', true); sevensegment.setSegmentState('f', true); sevensegment.setSegmentState('g', true); break;
         case 9: sevensegment.setSegmentState('a', true); sevensegment.setSegmentState('b', true); sevensegment.setSegmentState('c', true); sevensegment.setSegmentState('d', true); sevensegment.setSegmentState('f', true); sevensegment.setSegmentState('g', true); break;
      }
  }

  void SevenSegmentDigit::clearDigit() {
    for (char seg = 'a'; seg <= 'g'; ++seg) {
      sevensegment.setSegmentState(seg, false);
    }
 
  }

  void SevenSegmentDigit::display(GxEPD2_BW <WatchyDisplay, WatchyDisplay::HEIGHT> *watchDisplay) {
    // Draw the segments
    #ifdef DEBUG_SEVENSEGMENTDIGIT
    Serial.println("SevenSegmentDigit::display");
    #endif
    sevensegment.display(watchDisplay);
    #ifdef DEBUG_SEVENSEGMENTDIGIT
    Serial.println("SevenSegmentDigit::display done");
    #endif
  }

