#include "Watchy_QArtCode.h"
#include <stdlib.h>     //srand, rand

#include "../../sdk/UiSDK.h"

void showWatchFace_QArtCode(Watchy &watchy){

  UiSDK::initScreen(watchy.display);
  const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);
  const uint16_t bgColor = UiSDK::getWatchfaceBg(BASE_POLARITY);

    String image_string;

  UiSDK::fillScreen(watchy.display, bgColor);

    if(watchy.connectWiFi()){
        HTTPClient http;
        http.setConnectTimeout(3000);//3 second max timeout
        String imageURL = "https://qartcode.herokuapp.com/";
        http.begin(imageURL.c_str());
        int httpResponseCode = http.GET();
        if(httpResponseCode == 200) {
            Serial.print("Got 200 response");
            Serial.println();
            int bodyLen = http.getSize();
            Serial.print("Body size is ");
            Serial.println(bodyLen);
            image_string = http.getString();
            UiSDK::fillScreen(watchy.display, bgColor);

            int row = 0;
            int column = 0;

            for (int i=0; i <5000; i++) {
              for (int j=0; j < 8; j++) {
                int pixelValue = bitRead((int)image_string.charAt(i), 7 - j);
                if (pixelValue == 0) {
                  UiSDK::drawPixel(watchy.display, row, column, fgColor);
                } else {
                  UiSDK::drawPixel(watchy.display, row, column, bgColor);
                }
                row++;
                if (row % 200 == 0) {
                  row = 0;
                  column++;
                }
              }
            }

        }else{
            Serial.println("http error");
          UiSDK::drawBitmap(watchy.display, 0, 0, bitmap_broken, DISPLAY_WIDTH, DISPLAY_HEIGHT, fgColor);
        }
        http.end();

        WiFi.mode(WIFI_OFF);
        btStop();

    }else{
            Serial.println("no wifi");
          UiSDK::drawBitmap(watchy.display, 0, 0, bitmap_broken, DISPLAY_WIDTH, DISPLAY_HEIGHT, fgColor);
    }
    
    UiSDK::fillRect(watchy.display, 86, 88, 31, 12, fgColor);
        UiSDK::setTextColor(watchy.display, bgColor);
    UiSDK::setCursor(watchy.display, 87, 90);
    if(watchy.currentTime.Hour < 10){
        UiSDK::print(watchy.display, '0');
    }
    UiSDK::print(watchy.display, watchy.currentTime.Hour);
    UiSDK::print(watchy.display, ':');
    if(watchy.currentTime.Minute < 10){
        UiSDK::print(watchy.display, '0');
    }    
    UiSDK::print(watchy.display, watchy.currentTime.Minute);
    
    Serial.print("Oh and the time is ");
    if(watchy.currentTime.Hour < 10){
        Serial.print('0');
    }
    Serial.print(watchy.currentTime.Hour);
    Serial.print(':');
    if(watchy.currentTime.Minute < 10){
        Serial.print('0');
    }    
    Serial.print(watchy.currentTime.Minute);
    Serial.println();
}
