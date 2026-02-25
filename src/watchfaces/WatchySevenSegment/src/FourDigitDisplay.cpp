#include <Arduino.h>
#include <Wire.h>
#include <string.h>
#include <vector>
#include <map>

#include "../../../watchy/Watchy.h"
#include "../../../sdk/UiSDK.h"

#include "FourDigitDisplay.h"
#include "SevenSegmentDigit.h"

FourDigitDisplay::FourDigitDisplay(int x, int y, int width, int height) {
    #ifdef DEBUG_FOURDIGITDISPLAY
    Serial.println("FourDigitDisplay constructor start with " + String(x) + ", " + String(y) + ", " + String(width) + ", " + String(height));
    #endif
    this->x = x;
    this->y = y;
    this->width = width;
    this->height = height;

    int colon_width = 0;    
    if (colon) {
        colon_width = width / COLON_SCALE; // We'll snuggle up the digits a bit, to create some space in the middle for the colon
        colon_radius = colon_width / COLON_RADIUS_SCALE;
    }
    digit_width = (width / 4) - (colon_width /  4);
    digits[0] = SevenSegmentDigit(x, y, digit_width, height);
    digits[1] = SevenSegmentDigit(x + digit_width, y, digit_width, height);
    digits[2] = SevenSegmentDigit(x + width / 2 + colon_width / 2, y, digit_width, height);
    digits[3] = SevenSegmentDigit(x + width / 2 + colon_width / 2 + digit_width, y, digit_width, height);
    
    #ifdef DEBUG_FOURDIGITDISPLAY
    Serial.println("FourDigitDisplay constructor end");
    #endif
}   

void FourDigitDisplay::clearDisplay() {
    for (int i = 0; i < 4; i++) {
        digits[i].clearDigit();
    }
}

void FourDigitDisplay::setTwentyFourHour(bool state) {
    twenty_four_hour = state;
}

void FourDigitDisplay::setDigit(int digit, int number) {
    #ifdef DEBUG_FOURDIGITDISPLAY
    Serial.println("FourDigitDisplay::setDigit(" + String(digit) + ", " + String(number) + ")");
    #endif
    digits[digit].setNumber(number);
    #ifdef DEBUG_FOURDIGITDISPLAY
    Serial.println("Called setNumber and survived !");
    #endif        
}

void FourDigitDisplay::setTime(int hours, int minutes) {
    #ifdef DEBUG_FOURDIGITDISPLAY
    Serial.println("FourDigitDisplay::setTime(" + String(hours) + ", " + String(minutes) + ")");
    #endif
    if (twenty_four_hour) {
        setDigit(0, hours / 10);
        setDigit(1, hours % 10);
    } else {
        if (hours > 12)
        {
            hours -= 12;
        }
        if (hours == 0)
        {
            hours = 12;
        }
        if (hours >=10) {
            setDigit(0, hours / 10);
            setDigit(1, hours % 10);
        } else {
            digits[0].clearDigit(); // clear the first digit
            setDigit(1, hours );
        }
    }
    setDigit(2, minutes / 10);
    setDigit(3, minutes % 10);
    colon = true;  // We have a time to display.... so we'll display the colon
    decimal_point = false; // no decimal point for time
}

void FourDigitDisplay::setSWTime(int minutes, int seconds) {
    #ifdef DEBUG_FOURDIGITDISPLAY
    Serial.println("FourDigitDisplay::setSWTime(" + String(minutes) + ", " + String(seconds) + ")");
    #endif
    minutes = minutes % 100;
    seconds = seconds % 60;
    setDigit(0, minutes / 10);
    setDigit(1, minutes % 10);
    setDigit(2, seconds / 10);
    setDigit(3, seconds % 10);
    colon = true;  // We have a time to display.... so we'll display the colon
    decimal_point = false; // no decimal point for time
}

void FourDigitDisplay::setInt(int value) {
    #ifdef DEBUG_FOURDIGITDISPLAY
    Serial.println("FourDigitDisplay::setInt(" + String(value) +" ");
    #endif

    int digitValue= value / 1000;
    if (digitValue == 0) {
        digits[0].clearDigit();
    } else {
        setDigit(0, digitValue);
    }
    value=value % 1000;
    digitValue= value / 100;
    if (digitValue == 0) {
        digits[1].clearDigit();
    } else {
        setDigit(1, digitValue);
    }
    value=value % 100;
    digitValue= value / 10;
    if (digitValue == 0) {
        digits[2].clearDigit();
    } else {
        setDigit(2, digitValue);
    }
    digitValue= value % 10;
    setDigit(3, digitValue);
    decimal_point = false; 
    colon = false;
}

void FourDigitDisplay::setColon(bool state) {
    colon = state;
}

void FourDigitDisplay::setDecimalPoint(bool state) {
    decimal_point = state;
}

void FourDigitDisplay::display(GxEPD2_BW <WatchyDisplay, WatchyDisplay::HEIGHT> *watchDisplay) {
    const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);
    for (int i = 0; i < 4; i++) {
        digits[i].display(watchDisplay);
    }
    if (colon || decimal_point)
    {
        watchDisplay->fillCircle(x + width / 2, y + (height / 4) * 3, colon_radius, fgColor);
        if ( ! decimal_point ) {
            watchDisplay->fillCircle(x + width / 2, y + height / 4, colon_radius, fgColor);
        }
    }
}
