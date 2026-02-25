#include "Watchy_QR.h"

RTC_DATA_ATTR bool modeEZ = false;

void QR_Watchface::drawWatchFace(){

    //drawbg
    display.fillScreen(GxEPD_WHITE);

    //drawtime
    char time[20];
    sprintf(time, "%02d:%02d %d-%02d-%02d", currentTime.Hour, currentTime.Minute, tmYearToCalendar(currentTime.Year), currentTime.Month, currentTime.Day);
    QRCode qrcode;
    uint8_t qrcodeBytes[qrcode_getBufferSize(1)];
    qrcode_initText(&qrcode, qrcodeBytes, 1, ECC_MEDIUM, time);
    for (uint8_t y = 0; y < qrcode.size; y++) {
        for (uint8_t x = 0; x < qrcode.size; x++) {
            if(qrcode_getModule(&qrcode, x, y)){
                for (uint8_t i = 8*x; i< 8*x+8; i++){
                    for (uint8_t j = 8*y; j< 8*y+8; j++){
                        display.drawPixel(16+i, 16+j, GxEPD_BLACK);
                    }
                }
            } else {
                for (uint8_t i = 8*x; i< 8*x+8; i++){
                    for (uint8_t j = 8*y; j< 8*y+8; j++){
                        display.drawPixel(16+i, 16+j, GxEPD_WHITE);
                    }
                }
            }
        }
    }
    
    display.setCursor(5, 5);
    display.setTextColor(GxEPD_BLACK);
    if(modeEZ){
        display.print(time);
    }
}

void QR_Watchface::morseTime(){
    char time[5];
    sprintf(time, "%02d %02d", currentTime.Hour, currentTime.Minute);
    vibMorseString(String(time));
}

void QR_Watchface::vibMorseString(String s){
    unsigned int length = s.length();
    for(unsigned int i = 0; i<length; i++){
        vibMorseChar(s.charAt(i));
        vibMorseChar('+'); // Space between every char
    }
}


void QR_Watchface::vibMorseChar(char c){
    int TI_LENGHT = 100;
    std::map<int, std::vector<int>> morse_table;
    morse_table[' '] = {0}; // Space between words (must be 7, but it gets added to the space between letters before and after)
    morse_table['+'] = {0,0,0}; // Space between letters
    morse_table['0'] = {3,0,3,0,3,0,3,0,3};
    morse_table['1'] = {1,0,3,0,3,0,3,0,3};
    morse_table['2'] = {1,0,1,0,3,0,3,0,3};
    morse_table['3'] = {1,0,1,0,1,0,3,0,3};
    morse_table['4'] = {1,0,1,0,1,0,1,0,3};
    morse_table['5'] = {1,0,1,0,1,0,1,0,1};
    morse_table['6'] = {3,0,1,0,1,0,1,0,1};
    morse_table['7'] = {3,0,3,0,1,0,1,0,1};
    morse_table['8'] = {3,0,3,0,3,0,1,0,1};
    morse_table['9'] = {3,0,3,0,3,0,3,0,1};
    
    pinMode(VIB_MOTOR_PIN, OUTPUT);
    for(int i: morse_table[c]){
        if(i == 0){
            delay(TI_LENGHT);
        }else{
            digitalWrite(VIB_MOTOR_PIN, true);
            delay(i*TI_LENGHT);
            digitalWrite(VIB_MOTOR_PIN, false);
        }
    }
}

void QR_Watchface::handleButtonPress() {
    if (guiState == WATCHFACE_STATE) {
        uint64_t wakeupBit = esp_sleep_get_ext1_wakeup_status();

        if (wakeupBit & UP_BTN_MASK) {
            RTC.read(currentTime);
            showWatchFace(true);
            morseTime();
        }
        if (wakeupBit & DOWN_BTN_MASK) {
            modeEZ = !modeEZ;
            RTC.read(currentTime);
            showWatchFace(true);
        }
        if (wakeupBit & MENU_BTN_MASK) {
            Watchy::handleButtonPress();
            return;
        }
    } else {
        Watchy::handleButtonPress();
    }
    return;
}
