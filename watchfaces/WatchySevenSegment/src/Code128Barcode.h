#ifndef CODE128BARCODE_H
#define CODE128BARCODE_H

#include <Arduino.h>
#include "../../../src/watchy/Watchy.h"

class Code128Barcode {
    private:
        GxEPD2_BW <WatchyDisplay, WatchyDisplay::HEIGHT> *display;
        String text, encodedBarcode,label;
        bool codeC = false;    

    public:
        Code128Barcode(String text,String label, GxEPD2_BW <WatchyDisplay, WatchyDisplay::HEIGHT> *display);
        void encodeBarcodeC();
        void encodeBarcodeA();
        String encodeCode128C(int value);
        String encodeCode128A(int value);
      
        void draw();
};
#endif // CODE128BARCODE_H
