#ifndef QRBARCODE_H
#define QRBARCODE_H

#include <Arduino.h>
#include "../../../watchy/Watchy.h"

#define LOYALTY_QR_CODE_VERSION     1
#define LOYALTY_QR_CODE_PIXEL_SIZE  6

class QRBarcode {
    private:
        GxEPD2_BW <WatchyDisplay, WatchyDisplay::HEIGHT> *display;
        String text, label;
        int pixelSize = LOYALTY_QR_CODE_PIXEL_SIZE;
        QRCode qrcode;
        char stringToCreateCodeFor[90];
        int errorCorrection=ECC_MEDIUM;
        uint8_t *qrcodeBytes;   
    public:
        QRBarcode(String text, String label, GxEPD2_BW <WatchyDisplay, WatchyDisplay::HEIGHT> *display);
        void encodeBarcode();
        void draw();     

};

#endif // QRBARCODE_H