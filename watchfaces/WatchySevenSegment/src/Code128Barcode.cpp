#include <Arduino.h>
#include "../../../src/watchy/Watchy.h"
#include "../../../src/watchy/Display.h" 

#include "Code128Barcode.h"

Code128Barcode::Code128Barcode(String text,String label, GxEPD2_BW <WatchyDisplay, WatchyDisplay::HEIGHT> *display) {
    this->text = text;
    this->label = label;
    this->display = display;
    // test if text contains only digits and if so set codeC to true
    // code C will generate a more compact barcode
    if (text.length() % 2 == 0) {
        codeC = true;
        for (int i = 0; i < text.length(); i++) {
            if (text[i] < '0' || text[i] > '9') {
                codeC = false;
                break;
            }
        }
    }

    if (codeC) {
        encodeBarcodeC();
        return;
    } 
    encodeBarcodeA();
}
// routine to encode the barcode.... using Code C encoding member variable encodedBarcode should be set to a code 128 barcode
// encoded as a string of 1s and 0s, 1 where a line needs to be drawn, 0 where it should be white
void Code128Barcode::encodeBarcodeC(){
    // Encoding the barcode using Code 128 standard, Code C set
    encodedBarcode = "11010011100"; // Start code C for Code 128
    int checksum = 105; // Start with the value of the start code C
    for (int i = 0; i < text.length(); i += 2) {
        int value = (text[i] - '0') * 10 + (text[i + 1] - '0'); // Code C encodes pairs of digits
        checksum += value * ((i / 2) + 1);
        encodedBarcode += encodeCode128C(value); // Convert to Code 128 pattern
    }
    checksum %= 103; // Modulo 103 for the checksum
    encodedBarcode += encodeCode128C(checksum); // Append checksum
    encodedBarcode += "1100011101011"; // Stop code
}

void Code128Barcode::encodeBarcodeA(){
    encodedBarcode = "11010000100"; // Start code A for Code 128
    int checksum = 103; // Start with the value of the start code A 
    for (int i = 0; i < text.length(); i++) {
        int value = text[i] - ' '; // Code A encodes single values at a time
        checksum += value * (i + 1);
        encodedBarcode += encodeCode128A(value); // Convert to Code 128 pattern
    }
    checksum %= 103; // Modulo 103 for the checksum
    encodedBarcode += encodeCode128A(checksum); // Append checksum
    encodedBarcode += "1100011101011"; // Stop code
}

String Code128Barcode::encodeCode128C(int value) {
    const char* code128C[] = {
    "11011001100", "11001101100", "11001100110", "10010011000", "10010001100", "10001001100",
    "10011001000", "10011000100", "10001100100", "11001001000", "11001000100", "11000100100",
    "10110011100", "10011011100", "10011001110", "10111001100", "10011101100", "10011100110",
    "11001110010", "11001011100", "11001001110", "11011100100", "11001110100", "11101101110",
    "11101001100", "11100101100", "11100100110", "11101100100", "11100110100", "11100110010",
    "11011011000", "11011000110", "11000110110", "10100011000", "10001011000", "10001000110",
    "10110001000", "10001101000", "10001100010", "11010001000", "11000101000", "11000100010",
    "10110111000", "10110001110", "10001101110", "10111011000", "10111000110", "10001110110",
    "11101110110", "11010001110", "11000101110", "11011101000", "11011100010", "11011101110",
    "11101011000", "11101000110", "11100010110", "11101101000", "11101100010", "11100011010",
    "11101111010", "11001000010", "11110001010", "10100110000", "10100001100", "10010110000",
    "10010000110", "10000101100", "10000100110", "10110010000", "10110000100", "10011010000",
    "10011000010", "10000110100", "10000110010", "11000010010", "11001010000", "11110111010",
    "11000010100", "10001111010", "10100111100", "10010111100", "10010011110", "10111100100",
    "10011110100", "10011110010", "11110100100", "11110010100", "11110010010", "11011011110",
    "11011110110", "11110110110", "10101111000", "10100011110", "10001011110", "10111101000",
    "10111100010", "11110101000", "11110100010", "10111011110", "10111101110", "11101011110",
    "11110101110", "11010000100", "11010010000", "11010011100"
    };
    return String(code128C[value]);
}

String Code128Barcode::encodeCode128A(int value) {
    // Doh.... this is the same as code128C !!

    const char* code128A[] = {
    "11011001100", "11001101100", "11001100110", "10010011000", "10010001100", "10001001100",
    "10011001000", "10011000100", "10001100100", "11001001000", "11001000100", "11000100100",
    "10110011100", "10011011100", "10011001110", "10111001100", "10011101100", "10011100110",
    "11001110010", "11001011100", "11001001110", "11011100100", "11001110100", "11101101110",
    "11101001100", "11100101100", "11100100110", "11101100100", "11100110100", "11100110010",
    "11011011000", "11011000110", "11000110110", "10100011000", "10001011000", "10001000110",
    "10110001000", "10001101000", "10001100010", "11010001000", "11000101000", "11000100010",
    "10110111000", "10110001110", "10001101110", "10111011000", "10111000110", "10001110110",
    "11101110110", "11010001110", "11000101110", "11011101000", "11011100010", "11011101110",
    "11101011000", "11101000110", "11100010110", "11101101000", "11101100010", "11100011010",
    "11101111010", "11001000010", "11110001010", "10100110000", "10100001100", "10010110000",
    "10010000110", "10000101100", "10000100110", "10110010000", "10110000100", "10011010000",
    "10011000010", "10000110100", "10000110010", "11000010010", "11001010000", "11110111010",
    "11000010100", "10001111010", "10100111100", "10010111100", "10010011110", "10111100100",
    "10011110100", "10011110010", "11110100100", "11110010100", "11110010010", "11011011110",
    "11011110110", "11110110110", "10101111000", "10100011110", "10001011110", "10111101000",
    "10111100010", "11110101000", "11110100010", "10111011110", "10111101110", "11101011110",
    "11110101110", "11010000100", "11010010000", "11010011100"
    };

    return String(code128A[value]);
}

void Code128Barcode::draw() {
    int barWidth = 1;    
    int xStart = 100 - encodedBarcode.length()/2;     
    int yStart = 30;     
    int height = display->height() - yStart * 2;

    display->fillRect(0,0,display->width(), display->height(), GxEPD_WHITE); // clear the screen
    for (int i = 0; i < encodedBarcode.length(); i++) {
    if (encodedBarcode[i] == '1') {
        display->fillRect(xStart + (i * barWidth), yStart, barWidth, height, GxEPD_BLACK);
    } else {
        display->fillRect(xStart + (i * barWidth), yStart, barWidth, height, GxEPD_WHITE);
    }
    }
    // Now let's draw the label
    int16_t x1=0,y1=0;
    uint16_t textWidth,textHeight;

    display->setFont(&FreeMonoBold9pt7b);
    display->setTextColor(GxEPD_BLACK);
    display->getTextBounds(label.c_str(),0,0,&x1,&y1,&textWidth,&textHeight);
    display->fillRect(0,0,textWidth,textHeight,GxEPD_WHITE);
    display->setCursor(0,textHeight);
    display->print(label.c_str());

    // and let's print the barcode value at the bottom of the display
    display->getTextBounds(text.c_str(),0,0,&x1,&y1,&textWidth,&textHeight);
    display->fillRect(0,display->height()-textHeight,textWidth,textHeight,GxEPD_WHITE);
    display->setCursor(0,display->height()-1); // seems to clip the bottom of the text if we don't do this
    display->print(text.c_str());
}
