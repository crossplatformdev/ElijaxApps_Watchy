#include <Arduino.h>
#include <Wire.h>
#include <string.h>
#include <vector>

#include "TrapezoidSegment.h"

TrapezoidSegment::TrapezoidSegment(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4) {
        // Assign the given coordinates to the corner variables
        this->x1 = x1;
        this->y1 = y1;
        this->x2 = x2;
        this->y2 = y2;
        this->x3 = x3;
        this->y3 = y3;
        this->x4 = x4;
        this->y4 = y4;
        
        #ifdef DEBUG_TRAPEZOIDSEGMENT
        Serial.print("TrapezoidSegment created ");
        Serial.print(x1);
        Serial.print(", ");
        Serial.print(y1);
        Serial.print(", ");
        Serial.print(x2);
        Serial.print(", ");
        Serial.print(y2);
        Serial.print(", ");
        Serial.print(x3);
        Serial.print(", ");
        Serial.print(y3);
        Serial.print(", ");
        Serial.print(x4);
        Serial.print(", ");
        Serial.println(y4);
        #endif
}

int TrapezoidSegment::getX1() {
        return x1;
}

int TrapezoidSegment::getY1() {
        return y1;
}

int TrapezoidSegment::getX2() {
        return x2;
}

int TrapezoidSegment::getY2() {
        return y2;
}

int TrapezoidSegment::getX3() {
        return x3;
}

int TrapezoidSegment::getY3() {
        return y3;
}

int TrapezoidSegment::getX4() {
        return x4;
}

int TrapezoidSegment::getY4() {
        return y4;
}

