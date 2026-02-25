#include <Arduino.h>
#include "../../../watchy/Watchy.h"
#include "../../../watchy/Display.h" 
#include "../../../sdk/UiSDK.h"

#include "qrcode.h"
#include "QRBarcode.h"


QRBarcode::QRBarcode(String text, String label, GxEPD2_BW <WatchyDisplay, WatchyDisplay::HEIGHT> *display) {
    this->text = text;
    this->label = label;
    this->display = display;

    encodeBarcode();
}
void QRBarcode::encodeBarcode() { // fill the barcode 2 dimensional array, with 1s and 0s
    strcpy(stringToCreateCodeFor, text.c_str());
    qrcodeBytes = new uint8_t[qrcode_getBufferSize(LOYALTY_QR_CODE_VERSION)];
    qrcode_initText(&qrcode, qrcodeBytes, LOYALTY_QR_CODE_VERSION, errorCorrection, stringToCreateCodeFor );
}

void QRBarcode::draw() {
    const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);
    const uint16_t bgColor = UiSDK::getWatchfaceBg(BASE_POLARITY);

    int xoffset = (display->width() - qrcode.size * pixelSize) / 2;
    int yoffset = (display->height() - qrcode.size * pixelSize) / 2;
    for (uint8_t y = 0; y < qrcode.size; y++) {
        for (uint8_t x = 0; x < qrcode.size; x++) {
            if(qrcode_getModule(&qrcode, x, y)){
            display->fillRect(xoffset + x * pixelSize, yoffset + y * pixelSize, pixelSize, pixelSize, fgColor);
            } else {
            display->fillRect(xoffset + x * pixelSize, yoffset + y * pixelSize, pixelSize, pixelSize, bgColor);
            }
        }
    }
    // Now let's draw the label
    int16_t x1=0,y1=0;
    uint16_t textWidth,textHeight;

    display->setFont(&FreeMonoBold9pt7b);
    display->setTextColor(fgColor);
    display->getTextBounds(label.c_str(),0,0,&x1,&y1,&textWidth,&textHeight);
    display->fillRect(0,0,textWidth,textHeight,bgColor);
    display->setCursor(0,textHeight);
    display->print(label.c_str());

    // and let's print the barcode value at the bottom of the display
    display->getTextBounds(text.c_str(),0,0,&x1,&y1,&textWidth,&textHeight);
    display->fillRect(0,display->height()-textHeight,textWidth,textHeight,bgColor);
    display->setCursor(0,display->height()-1); // seems to clip the bottom of the text if we don't do this
    display->print(text.c_str());

}

