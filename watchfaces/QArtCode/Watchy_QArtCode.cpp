#include "Watchy_QArtCode.h"
#include <stdlib.h>     //srand, rand

WatchyQArtCode::WatchyQArtCode(){} //constructor

void WatchyQArtCode::drawWatchFace(){
    String image_string;
    bool drawSuccess = false;

    display.fillScreen(GxEPD_BLACK);
    // display.setCursor(0, 24);
    // display.println("INITIALISED!");

    if(connectWiFi()){
        HTTPClient http;
        http.setConnectTimeout(2000);
        http.setTimeout(3000);
        String imageURL = "https://qartcode.herokuapp.com/";
        
        if(http.begin(imageURL.c_str())){
            int httpResponseCode = http.GET();
            if(httpResponseCode == 200) {
                Serial.print("Got 200 response");
                Serial.println();
                int bodyLen = http.getSize();
                Serial.print("Body size is ");
                Serial.println(bodyLen);
                image_string = http.getString();
                
                if(image_string.length() > 0){
                    display.fillScreen(GxEPD_WHITE);
                    int row = 0;
                    int column = 0;
                    for (int i=0; i < 5000 && i < image_string.length(); i++) {
                      for (int j=0; j < 8; j++) {
                        int pixelValue = bitRead((int)image_string.charAt(i), 7 - j);
                        if (pixelValue == 0) {
                          display.drawPixel(row, column, GxEPD_BLACK);
                        } else {
                          display.drawPixel(row, column, GxEPD_WHITE);
                        }
                        row++;
                        if (row % 200 == 0) {
                          row = 0;
                          column++;
                        }
                      }
                    }
                    drawSuccess = true;
                }
            }else{
                Serial.println("http error: ");
                Serial.println(httpResponseCode);
            }
        }
        http.end();
        WiFi.mode(WIFI_OFF);
        btStop();
    }else{
        Serial.println("no wifi");
    }
    
    if(!drawSuccess){
        display.drawBitmap(0, 0, bitmap_broken, DISPLAY_WIDTH, DISPLAY_HEIGHT, GxEPD_WHITE);
    }
    
    // print time
    display.fillRect(86, 88, 31, 12, GxEPD_BLACK);
    // no! too big! Back to default of 9pt
    // display.setFont(&FreeMonoBold12pt7b);
    display.setTextColor(GxEPD_WHITE);
    display.setCursor(87, 90);
    if(currentTime.Hour < 10){
        display.print('0');
    }
    display.print(currentTime.Hour);
    display.print(':');
    if(currentTime.Minute < 10){
        display.print('0');
    }    
    display.print(currentTime.Minute);
    
    Serial.print("Oh and the time is ");
    if(currentTime.Hour < 10){
        Serial.print('0');
    }
    Serial.print(currentTime.Hour);
    Serial.print(':');
    if(currentTime.Minute < 10){
        Serial.print('0');
    }    
    Serial.print(currentTime.Minute);
    Serial.println();

    display.display(false);
}
