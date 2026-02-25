#include "Watchy_Squarbital.h"
#include "../../../sdk/UiSDK.h"

RTC_DATA_ATTR static int mode = 0;

/* Values of voltage considered as max (100%) and min (0%), to display the central circle.
 * Value is in milli-volt.
 * The default value is usually 3800 to 4100, but I found that my battery was more between 3500 and 4200. YMMV, hence the setting.
 */
int batteryVMax = 4200;
int batteryVMin = 3500;

void Squarbital_Watchface::drawWatchFace(){

    UiSDK::initScreen(display);
    primaryColor = UiSDK::getWatchfaceBg(BASE_POLARITY);
    secondaryColor = UiSDK::getWatchfaceFg(BASE_POLARITY);
    twentyFour = (mode == 1);
    
    printDecor();
    drawSquare(100, 8, 10, 360.0*currentTime.Minute/60.0);
    
    float angleHour;
    if(twentyFour){
        angleHour = 360.0*currentTime.Hour/24.0;
    }else{
        angleHour = 360.0*(currentTime.Hour%12)/12.0;
    }
    drawSquare(100, 30, 10, angleHour);
    
    float nbDays = (float)daysPerMonth(tmYearToCalendar(currentTime.Year), currentTime.Month);
    drawSquare(100, 52, 10, 360.0*currentTime.Day/nbDays);
    drawSquare(100, 74, 10, 360.0*currentTime.Month/12.0);
    drawSquare(100, 86, 14, getBatteryAngle());
}


void Squarbital_Watchface::printDecor(){
    UiSDK::fillScreen(display, primaryColor);
    UiSDK::fillRect(display, 6, 6, 188, 188, secondaryColor);
    UiSDK::fillRect(display, 20, 20, 160, 160, primaryColor);
    UiSDK::fillRect(display, 28, 28, 144, 144, secondaryColor);
    UiSDK::fillRect(display, 42, 42, 116, 116, primaryColor);
    UiSDK::fillRect(display, 50, 50, 100, 100, secondaryColor);
    UiSDK::fillRect(display, 64, 64, 72, 72, primaryColor);
    UiSDK::fillRect(display, 72, 72, 56, 56, secondaryColor);
}

/**
 * 
 * (x,y) coordinate of the top-left corner of the square, at 0 angle. x must be 100, usually
 * width the width of the square
 * angle angle of end of the square, in degrees.wh
 * 
 */
void Squarbital_Watchface::drawSquare(int x, int y, int width, float angle){
    float DEG2RAD = 0.0174532925;
    
    while(angle > 360){
        angle -= 360;
    }
    
    if(angle >= 45 || angle == 360){
        UiSDK::fillRect(display, x, y, 100-y, width, primaryColor);
    }
    if(angle >= 135 || angle == 360){
        UiSDK::fillRect(display, (200-y)-width, y, width, 200-2*y, primaryColor);
    }
    if(angle >= 225 || angle == 360){
        UiSDK::fillRect(display, y, (200-y)-width, 200-2*y, width, primaryColor);
    }
    if(angle >= 315 || angle == 360){
        UiSDK::fillRect(display, y, y, width, 200-2*y, primaryColor);
    }
    
    //x(a)=100+sin(a)*((half square length)/max(abs(cos(a));abs(sin(a))))
    //y(a)=100-cos(-a)*((half square length)/max(abs(cos(-a));abs(sin(-a))))
    if(angle == 360){
        UiSDK::fillRect(display, y, y, 100-y, width, primaryColor);
    }else if(angle < 45){
        float xEnd = 100.0+sin(angle*DEG2RAD)*(100-y)/(float)max(abs(cos(angle*DEG2RAD)), abs(sin(angle*DEG2RAD)));
        UiSDK::fillRect(display, x, y, xEnd-x, width, primaryColor);
        float xSmallEnd = 100.0+sin(angle*DEG2RAD)*((100-y)-width)/(float)max(abs(cos(angle*DEG2RAD)), abs(sin(angle*DEG2RAD)));
        UiSDK::fillTriangle(display, xEnd+1, y, xEnd+1, y+width, xSmallEnd+1, y+width, secondaryColor);
    }else if(angle < 135){
        float yEnd = 100.0-(cos(-angle*DEG2RAD))*((100-y)/(float)max(abs(cos(-angle*DEG2RAD)), abs(sin(-angle*DEG2RAD))));
        UiSDK::fillRect(display, (200-y)-width, y, width, yEnd-y, primaryColor);
        float ySmallEnd = 100.0-(cos(-angle*DEG2RAD))*(((100-y)-width)/(float)max(abs(cos(-angle*DEG2RAD)), abs(sin(-angle*DEG2RAD))));
        if(angle<90){
            UiSDK::fillTriangle(display, (200-y)-width, yEnd, 200-y-1, yEnd, (200-y)-width, ySmallEnd+1, primaryColor);
            UiSDK::fillTriangle(display, (200-y)-width, ySmallEnd+2, 200-y-1, ySmallEnd+2, 200-y-1, yEnd+1, secondaryColor);
        }else{
            UiSDK::fillTriangle(display, (200-y)-width, yEnd+1, 200-y, yEnd+1, (200-y)-width, ySmallEnd, secondaryColor);
        }     
    }else if(angle < 225){
        float xEnd = 100.0+sin(angle*DEG2RAD)*(100-y)/(float)max(abs(cos(angle*DEG2RAD)), abs(sin(angle*DEG2RAD)));
        UiSDK::fillRect(display, xEnd, (200-y)-width, (200-y)-xEnd, width, primaryColor);
        float xSmallEnd = 100.0+sin(angle*DEG2RAD)*((100-y)-width)/(float)max(abs(cos(angle*DEG2RAD)), abs(sin(angle*DEG2RAD)));
        if(angle<180){
            UiSDK::fillTriangle(display, xEnd, 200-y, xEnd, (200-y)-width, xSmallEnd-1, (200-y)-width, primaryColor);
            UiSDK::fillTriangle(display, xEnd-1, 200-y, xSmallEnd-2, (200-y)-width, xSmallEnd-2, 200-y, secondaryColor);
        }else{
            UiSDK::fillTriangle(display, xEnd-1, 200-y, xEnd-1, (200-y)-width, xSmallEnd-1, (200-y)-width, secondaryColor);
        }
    }else if(angle < 315){
        float yEnd = 100.0-(cos(-angle*DEG2RAD))*((100-y)/(float)max(abs(cos(-angle*DEG2RAD)), abs(sin(-angle*DEG2RAD))));
        UiSDK::fillRect(display, y, yEnd, width, (200-y)-yEnd, primaryColor);
        float ySmallEnd = 100.0-(cos(-angle*DEG2RAD))*(((100-y)-width)/(float)max(abs(cos(-angle*DEG2RAD)), abs(sin(-angle*DEG2RAD))));
        if(angle<270){
            UiSDK::fillTriangle(display, y, yEnd, y+width-1, yEnd, y+width-1, ySmallEnd-1, primaryColor);
            UiSDK::fillTriangle(display, y, yEnd, y+width-1, ySmallEnd-2, y, ySmallEnd-2, secondaryColor);
        }else{
            UiSDK::fillTriangle(display, y, yEnd-1, y+width, yEnd-1, y+width, ySmallEnd, secondaryColor);
        }
    }else{
        float xEnd = 100.0+sin(angle*DEG2RAD)*(100-y)/(float)max(abs(cos(angle*DEG2RAD)), abs(sin(angle*DEG2RAD)));
        UiSDK::fillRect(display, y, y, xEnd-y, width, primaryColor);
        float xSmallEnd = 100.0+sin(angle*DEG2RAD)*((100-y)-width)/(float)max(abs(cos(angle*DEG2RAD)), abs(sin(angle*DEG2RAD)));
        UiSDK::fillTriangle(display, xEnd, y, xEnd, y+width, xSmallEnd, y+width, primaryColor);
        UiSDK::fillTriangle(display, xEnd+1, y, xSmallEnd+1, y+width, xSmallEnd+1, y, secondaryColor);
    }
}

int Squarbital_Watchface::daysPerMonth(int year, int month) {
    if (month == 2) {
        if (year % 4 != 0 || (year % 100 == 0 && year % 400 != 0)) {
            return 28;
        }else {
            return 29;
        }
    }else if (month == 4 || month == 6 || month == 9 || month == 11) {
        return 30;
    }else {
        return 31;
    }
}

float Squarbital_Watchface::getBatteryAngle() {
    int voltage = analogReadMilliVolts(BATT_ADC_PIN) * 2;
    float batteryAngle = 360*(voltage - batteryVMin) / (float)(batteryVMax - batteryVMin);
    if(batteryAngle < 0.0){
        batteryAngle = 0.0;
    }else if(batteryAngle > 360.0){
        batteryAngle = 360.0;
    }
    return batteryAngle;
}

void Squarbital_Watchface::morseTime(){
    char time[5];
    sprintf(time, "%02d %02d", currentTime.Hour, currentTime.Minute);
    vibMorseString(String(time));
}

void Squarbital_Watchface::vibMorseString(String s){
    unsigned int length = s.length();
    for(unsigned int i = 0; i<length; i++){
        vibMorseChar(s.charAt(i));
        vibMorseChar('+'); // Space between every char
    }
}


void Squarbital_Watchface::vibMorseChar(char c){
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

void Squarbital_Watchface::handleButtonPress() {
    if (guiState == WATCHFACE_STATE) {
        uint64_t wakeupBit = esp_sleep_get_ext1_wakeup_status();

        if (wakeupBit & UP_BTN_MASK) {
            RTC.read(currentTime);
            showWatchFace(true);
            morseTime();
        }
        if (wakeupBit & DOWN_BTN_MASK) {
            mode = (mode + 1) % 2;
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

void showWatchFace_Squarbital(Watchy &watchy) {
    Squarbital_Watchface face(watchy.settings);
    face.currentTime = watchy.currentTime;
    face.drawWatchFace();
}
