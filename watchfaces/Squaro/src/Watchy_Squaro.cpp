#include "Watchy_Squaro.h"
#include "Squaro_digits.h"

RTC_DATA_ATTR bool darkTheme = false;

void Squaro_Watchface::drawWatchFace(){

    if(darkTheme){
        primaryColor = GxEPD_WHITE;
        secondaryColor = GxEPD_BLACK;
    }else{
        primaryColor = GxEPD_BLACK;
        secondaryColor = GxEPD_WHITE;
    }
    
    display.fillScreen(secondaryColor);

    display.drawBitmap(2, 2, squaro_allArray[currentTime.Hour/10], 95, 95, primaryColor);
    display.drawBitmap(102, 2, squaro_allArray[currentTime.Hour%10], 95, 95, primaryColor);
    display.drawBitmap(2, 102, squaro_allArray[currentTime.Minute/10], 95, 95, primaryColor);
    display.drawBitmap(102, 102, squaro_allArray[currentTime.Minute%10], 95, 95, primaryColor);
}

void Squaro_Watchface::morseTime(){
    char time[5];
    sprintf(time, "%02d %02d", currentTime.Hour, currentTime.Minute);
    vibMorseString(String(time));
}

void Squaro_Watchface::vibMorseString(String s){
    unsigned int length = s.length();
    for(unsigned int i = 0; i<length; i++){
        vibMorseChar(s.charAt(i));
        vibMorseChar('+'); // Space between every char
    }
}

void Squaro_Watchface::vibMorseChar(char c){
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

void Squaro_Watchface::handleButtonPress() {
    if (guiState == WATCHFACE_STATE) {
        uint64_t wakeupBit = esp_sleep_get_ext1_wakeup_status();

        if (wakeupBit & UP_BTN_MASK) {
            RTC.read(currentTime);
            showWatchFace(true);
            morseTime();
        }
        if (wakeupBit & DOWN_BTN_MASK) {
            darkTheme = !darkTheme;
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
