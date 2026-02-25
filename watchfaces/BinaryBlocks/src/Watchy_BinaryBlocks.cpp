#include "Watchy_BinaryBlocks.h"

RTC_DATA_ATTR int mode = 0;

void BinaryBlocks_Watchface::drawWatchFace(){

    /* 4 modes
     */
    if(mode==0 || mode==1){
        primaryColor = GxEPD_BLACK;
        secondaryColor = GxEPD_WHITE;
    }else{
        primaryColor = GxEPD_WHITE;
        secondaryColor = GxEPD_BLACK;
    }
    if(mode==0 || mode==2){
        hands = true;
    }else{
        hands = false;
    }
    
    display.fillScreen(secondaryColor);
    
    display.fillRect(16, 16, 170, 170, primaryColor);
    display.fillRect(61, 61, 80, 80, secondaryColor);
    
    int hourD1 = currentTime.Hour/10;
    int hourD2 = currentTime.Hour%10;
    int minuteD1 = currentTime.Minute/10;
    int minuteD2 = currentTime.Minute%10;
    
    if(hourD1==1){
        display.fillRect(61, 109, 16, 16, primaryColor);
    }
    if(hourD1==2){
        display.fillRect(61, 77, 16, 16, primaryColor);
    }
    
    if(hourD2/8 >= 1){
        display.fillRect(125, 63, 16, 16, primaryColor);
    }
    if((hourD2%8)/4 >= 1){
        display.fillRect(125, 83, 16, 16, primaryColor);
    }
    if((hourD2%4)/2 >= 1){
        display.fillRect(125, 103, 16, 16, primaryColor);
    }
    if(hourD2%2 == 1){
        display.fillRect(125, 123, 16, 16, primaryColor);
    }
    
    
    if(minuteD1/4 >= 1){
        display.fillRect(16, 60, 16, 16, secondaryColor);
    }
    if((minuteD1%4)/2 >= 1){
        display.fillRect(16, 92, 16, 16, secondaryColor);
    }
    if(minuteD1%2 == 1){
        display.fillRect(16, 124, 16, 16, secondaryColor);
    }
    
    if(minuteD2/8 >= 1){
        display.fillRect(170, 44, 16, 16, secondaryColor);
    }
    if((minuteD2%8)/4 >= 1){
        display.fillRect(170, 76, 16, 16, secondaryColor);
    }
    if((minuteD2%4)/2 >= 1){
        display.fillRect(170, 108, 16, 16, secondaryColor);
    }
    if(minuteD2%2 == 1){
        display.fillRect(170, 140, 16, 16, secondaryColor);
    }
    
    //x(a)=100+sin(a)*((half square length)/max(abs(cos(a));abs(sin(a))))
    //y(a)=100-cos(-a)*((half square length)/max(abs(cos(-a));abs(sin(-a))))
    if(hands){
        float DEG2RAD = 0.0174532925;
        // Hours
        float hourAngle = DEG2RAD*(360.0*((float)currentTime.Hour+(float)currentTime.Minute/60.0)/12.0);
        float hourX = 100.0+sin(hourAngle)*47.0/(float)max(abs(cos(hourAngle)), abs(sin(hourAngle)));
        float hourY = 100.0-(cos(-hourAngle))*(47.0/(float)max(abs(cos(-hourAngle)), abs(sin(-hourAngle)) ));
        display.fillCircle(hourX, hourY, 7, secondaryColor);
        
        // Minutes
        float minuteAngle = DEG2RAD*(360.0*(float)currentTime.Minute/60.0);
        float minutesX = 100.0+sin(minuteAngle)*92.0/(float)max(abs(cos(minuteAngle)), abs(sin(minuteAngle)));
        float minutesY = 100.0-cos(-minuteAngle)*92.0/(float)max(abs(cos(-minuteAngle)), abs(sin(-minuteAngle)));
        display.fillCircle(minutesX, minutesY, 7, primaryColor);
    }
}

void BinaryBlocks_Watchface::morseTime(){
    char time[5];
    sprintf(time, "%02d %02d", currentTime.Hour, currentTime.Minute);
    vibMorseString(String(time));
}

void BinaryBlocks_Watchface::vibMorseString(String s){
    unsigned int length = s.length();
    for(unsigned int i = 0; i<length; i++){
        vibMorseChar(s.charAt(i));
        vibMorseChar('+'); // Space between every char
    }
}


void BinaryBlocks_Watchface::vibMorseChar(char c){
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

void BinaryBlocks_Watchface::handleButtonPress() {
    if (guiState == WATCHFACE_STATE) {
        uint64_t wakeupBit = esp_sleep_get_ext1_wakeup_status();

        if (wakeupBit & UP_BTN_MASK) {
            RTC.read(currentTime);
            showWatchFace(true);
            morseTime();
        }
        if (wakeupBit & DOWN_BTN_MASK) {
            mode += 1;
            if(mode == 4) mode = 0;
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
