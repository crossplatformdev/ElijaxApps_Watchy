#include "watchy_orbital.h"
#include "watchy_orbital_settings.h"

void WatchyOrbital::drawWatchFace() {
    /* 8 modes
     * 0 : white on black, no details, 12h
     * 1 : white on black, no details, 24h
     * 2 : white on black, details, 12h
     * 3 : white on black, details, 24h
     * 4 : black on white, no details, 12h
     * 5 : black on white, no details, 24h
     * 6 : black on white, details, 12h
     * 7 : black on white, details, 24h
     */
    if(mode==0 || mode==1 || mode==2 || mode==3){
        primaryColor = GxEPD_BLACK;
        secondaryColor = GxEPD_WHITE;
    }else{
        primaryColor = GxEPD_WHITE;
        secondaryColor = GxEPD_BLACK;
    }
    if(mode==0 || mode==1 || mode==4 || mode==5){
        details = false;
    }else{
        details = true;
    }
    if(mode==1 || mode==3 || mode==5 || mode==7){
        twentyFour = true;
    }else{
        twentyFour = false;
    }

    display.fillScreen(primaryColor);
    display.setTextWrap(false);
    sun.setPosition(lat, lon, settings.gmtOffset / 3600);
    sun.setCurrentDate(tmYearToCalendar(currentTime.Year), currentTime.Month, currentTime.Day);
    display.setTextColor(secondaryColor);
    drawMoon();
    drawWatchTime();
    drawBattery();
    drawDayTime();
    drawWeather();
}


void WatchyOrbital::drawWatchTime() {
    drawWatchMinute();
    drawWatchHour();
    drawWatchDay();
    drawWatchMonth();

    display.setFont(&FreeSansBold9pt7b);
    display.setCursor(0, 13);
    display.print(dayStr(currentTime.Wday));

    /*display.setCursor(159, 195);
    display.println(tmYearToCalendar(currentTime.Year));*/
}


/*
 * Draw minute circle
 */
void WatchyOrbital::drawWatchMinute() {
    float minuteAngle = 360.0 * (currentTime.Minute / 60.0);
    int tick = 360 / 60;

    fillArc3(0.0, minuteAngle, 88, 13, secondaryColor, 1);

    fillArc2(tick, minuteAngle - tick, 88, 13, primaryColor, tick);
    fillArc2(minuteAngle + (details?2*tick:tick), 360 - tick, 85, 7, secondaryColor, tick);
    
    if(details){
        drawDetails(minuteAngle, 88, String(currentTime.Minute));
    }
}


/*
 * Display hour circle
 * 24h or 12h, the latter to follow a classic watch
 */
void WatchyOrbital::drawWatchHour() {
    //((currentTime.Hour+11)%12)+1;
    float hourAngle;
    if(twentyFour){
        hourAngle = 360.0 * (currentTime.Hour / 24.0);
    }else{
        int hour = currentTime.Hour;
        if(hour>=12){
            hour -= 12;
        }
        hourAngle = 360.0 * (hour / 12.0);
    }
    int tick = 360 / (twentyFour?24:12);

    // Draw arcs: black arc, white seperator, black seperators
    fillArc3(0.0, hourAngle, 68, 15, secondaryColor, 1);

    fillArc2(tick, hourAngle - tick, 68, 15, primaryColor, tick);
    fillArc2(hourAngle + tick, 360 - tick, 64, 8, secondaryColor, tick);
    
    if(details){
        drawDetails(hourAngle, 68, String(currentTime.Hour));
    }
}


/*
 * Display day circle
 */
void WatchyOrbital::drawWatchDay() {
    float dayAngle = 360.0 * (currentTime.Day / 31.0);
    int tick = 360 / DaysPerMonth(currentTime.Year, currentTime.Month);

    // Draw arcs: black arc, white seperator, black seperators
    fillArc3(0.0, dayAngle, 42, 15, secondaryColor, 2);
    if (dayAngle == 360.0) {
        fillArc2(tick, dayAngle, 42, 15, primaryColor, tick);
    }
    else {
        fillArc2(tick, dayAngle - tick, 42, 15, primaryColor, tick);
        fillArc2(dayAngle + (details?2*tick:tick), 360 - tick, 40, 7, secondaryColor, tick);
    }
    
    if(details){
        drawDetails(dayAngle, 42, String(currentTime.Day));
    }
}


/*
 * Display the month circle
 */
void WatchyOrbital::drawWatchMonth() {
    float monthAngle = 360.0 * (currentTime.Month / 12.0);
    int tick = 360 / 12;

    // Draw arcs: black arc, white seperator, black seperators
    fillArc3(0.0, monthAngle, 22, 17, secondaryColor, 2);
    if (monthAngle == 360.0) {
        fillArc2(tick, monthAngle, 22, 17, primaryColor, tick);
    }
    else {
        fillArc2(tick, monthAngle - tick, 22, 17, primaryColor, tick);
        fillArc2(monthAngle + (details?2*tick:tick), 360 - tick, 20, 8, secondaryColor, tick);
    }
    
    if(details){
        drawDetails(monthAngle, 22, shortMonths[currentTime.Month-1]);
    }
}

/*
 * Display the battery level as a cricle in the middle
 * Percentage calculated with batteryVMin (0%) < voltage < batteryVMax (100%)
 */
void WatchyOrbital::drawBattery() {
    int voltage = analogReadMilliVolts(BATT_ADC_PIN) * 2;
    float batteryAngle = 360*(voltage - batteryVMin) / (float)(batteryVMax - batteryVMin);
    if(batteryAngle < 0.0){
        batteryAngle = 0.0;
    }else if(batteryAngle > 360.0){
        batteryAngle = 360.0;
    }
    fillArc3(0.0, batteryAngle, 4, 8, secondaryColor, 1);
    
    /*if(details){
        drawDetails(batteryAngle, 4, String(voltage));
    }*/
}

/*
 * Draw the phase of the moon
 */
void WatchyOrbital::drawMoon() {
    long long unix_time = (long long)makeTime(currentTime);
    int moonphase = sun.moonPhase(unix_time);

    int moonX = 4;
    int moonY = 196;

    display.setFont(&MoonPhases20pt7b);
    display.setCursor(moonX, moonY);
    display.print((char)(moonphase+0x50));
}


/*
 * Display the span of time the sun is up on the hour circle
 * If in details mode and in the right hour, it display it in the minute circle too (thinly)
 */
void WatchyOrbital::drawDayTime() {
    float sunrise = sun.calcSunrise();
    float sunset = sun.calcSunset();
    float sunriseAngle;
    float sunsetAngle;
    if(twentyFour){
        sunriseAngle = sunrise / 1440 * 360;
        sunsetAngle = sunset / 1440 * 360;

        fillArc3(sunriseAngle, sunsetAngle, 55, 2, secondaryColor, 1);
    }else{
        sunriseAngle = sunrise / 720 * 360;
        sunsetAngle = (sunset / 720 * 360)-360;
        if(currentTime.Hour<12){
            fillArc3(sunriseAngle, 360, 55, 2, secondaryColor, 1);
        }else{
            fillArc3(0, sunsetAngle, 55, 2, secondaryColor, 1);
        }
    }
    if(details){
        if((int)floor(sunrise/60) == currentTime.Hour){
            float minuteAngle = 360*((float)(((int)sunrise) % 60)/60);
            fillArc3(minuteAngle, 360, 79, 1, secondaryColor, 1);
        }else if((int)floor(sunset/60) == currentTime.Hour){
            float minuteAngle = 360*((float)(((int)sunset) % 60)/60);
            fillArc3(0, minuteAngle, 79, 1, secondaryColor, 1);
        }
    }
}


/*
 * Display icon+temp if wifi available.
 * Display the internal temp in any case
 */
void WatchyOrbital::drawWeather(){
    weatherData currentWeather = getWeatherData();

    if(currentWeather.external){
        char icon = 'A';
        int16_t weatherCode = currentWeather.weatherConditionCode;
        //A,  B,  C,  D,  E,  F,  G,  H,  I
        //01d,02d,03d,04d,09d,10d,11d,13d,50d
        //https://openweathermap.org/weather-conditions
        if(weatherCode >= 200 && weatherCode <= 232){
            //11d Thunderstorm
            icon = 'G';
        }else if((weatherCode >= 300 && weatherCode <= 321) || (weatherCode >= 520 && weatherCode <= 531)){
            //09d Drizzle
            icon = 'E';
        }else if(weatherCode >= 500 && weatherCode <= 504){
            //10d Rain
            icon = 'F';
        }else if(weatherCode == 511 || (weatherCode >= 600 && weatherCode <= 622)){
            //13d Snow
            icon = 'H';
        }else if(weatherCode >= 701 && weatherCode <= 781){
            //50d Atmo (Mist/Smoke...)
            icon = 'I';
        }else if(weatherCode == 800){
            //01d Clear
            icon = 'A';
        }else if(weatherCode == 801){
            //02d Few clouds
            icon = 'B';
        }else if(weatherCode == 802){
            //03d Scattered clouds
            icon = 'C';
        }else if(weatherCode == 803 || weatherCode == 804){
            //04d Overcast
            icon = 'D';
        }
        // Nighttime, switch to lowercase
        if(currentTime.Hour < 8 || currentTime.Hour >= 20){
            icon = icon + 0x20;
        }
        display.setFont(&OpenWeather_Regular12pt7b);
        int16_t x1;
        int16_t y1;
        uint16_t width;
        uint16_t height;
        display.getTextBounds(String(icon), 0, 0, &x1, &y1, &width, &height);
        display.setCursor(200-36, 27);
        display.print(icon);
        
        display.setFont(&FreeSansBold9pt7b);
        display.setCursor(165, 180);
        display.print(String(currentWeather.temperature)+((settings.weatherUnit == "metric")?" C":" F"));
    }
    display.setFont(&FreeSansBold9pt7b);
    display.setCursor(165, 195);
    display.print(sensor.readTemperature());
}


void WatchyOrbital::fillArc2(float start_angle, float end_angle, unsigned int radius, unsigned int width, unsigned int colour, float step) {
    unsigned int center_x = 100;
    unsigned int center_y = 100;

    // Draw colour blocks every inc degrees
    for (float i = start_angle; i <= end_angle; i += step) {

        float cx1 = center_x + (radius - width / 2) * cos((i - 90) * DEG2RAD);
        float cy1 = center_y + (radius - width / 2) * sin((i - 90) * DEG2RAD);
        float cx2 = center_x + (radius + width / 2) * cos((i - 90) * DEG2RAD);
        float cy2 = center_y + (radius + width / 2) * sin((i - 90) * DEG2RAD);

        display.drawLine(cx1, cy1, cx2, cy2, colour);
    }
}


//the triangles version
void WatchyOrbital::fillArc3(float start_angle, float end_angle, unsigned int radius, unsigned int width, unsigned int colour, float step) {
    unsigned int center_x = 100;
    unsigned int center_y = 100;

    // Draw colour blocks every inc degrees
    for (float i = start_angle; i <= end_angle - step; i += step) {
        float cx1 = center_x + (radius - width / 2) * cos((i - 90) * DEG2RAD);
        float cy1 = center_y + (radius - width / 2) * sin((i - 90) * DEG2RAD);
        float cx2 = center_x + (radius + width / 2) * cos((i - 90) * DEG2RAD);
        float cy2 = center_y + (radius + width / 2) * sin((i - 90) * DEG2RAD);
        float cx3 = center_x + (radius - width / 2) * cos((i - 90 + step) * DEG2RAD);
        float cy3 = center_y + (radius - width / 2) * sin((i - 90 + step) * DEG2RAD);
        float cx4 = center_x + (radius + width / 2) * cos((i - 90 + step) * DEG2RAD);
        float cy4 = center_y + (radius + width / 2) * sin((i - 90 + step) * DEG2RAD);

        display.fillTriangle(cx1, cy1, cx2, cy2, cx3, cy3, colour);
        display.fillTriangle(cx2, cy2, cx3, cy3, cx4, cy4, colour);
    }
}

unsigned int WatchyOrbital::DaysPerMonth(unsigned int year, unsigned int month) {
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


// https://github.com/adafruit/Adafruit-GFX-Library/pull/217 code by Soyunr - contributed under BSD license
void WatchyOrbital::drawEllipse(int16_t x0, int16_t y0, int16_t rx, int16_t ry, uint16_t color) {
    int16_t Rx2 = rx * rx;
    int16_t Ry2 = ry * ry;
    int16_t twoRx2 = 2 * Rx2;
    int16_t twoRy2 = 2 * Ry2;
    int16_t p;
    int16_t x = 0;
    int16_t y = ry;
    int16_t px = 0;
    int16_t py = twoRx2 * y;
    display.startWrite();
    display.writePixel(x0 + x, y0 + y, color);
    display.writePixel(x0 - x, y0 + y, color);
    display.writePixel(x0 + x, y0 - y, color);
    display.writePixel(x0 - x, y0 - y, color);
    //region1
    p = (int16_t)(Ry2 - Rx2 * ry + 0.25 * Rx2);
    while (px < py) {
        x++;
        px += twoRy2;
        if (p < 0)
            p += Ry2 + px;
        else {
            y--;
            py -= twoRx2;
            p += Ry2 + px - py;
        }
        display.writePixel(x0 + x, y0 + y, color);
        display.writePixel(x0 - x, y0 + y, color);
        display.writePixel(x0 + x, y0 - y, color);
        display.writePixel(x0 - x, y0 - y, color);
    }
    //region2
    p = (int16_t)(Ry2 * (x + 0.5) * (x + 0.5) + Rx2 * (y - 1) * (y - 1) - Rx2 * Ry2);
    while (y > 0) {
        y--;
        py -= twoRx2;
        if (p > 0)
            p += Rx2 - py;
        else {
            x++;
            px += twoRy2;
            p += Rx2 - py + px;
        }
        display.writePixel(x0 + x, y0 + y, color);
        display.writePixel(x0 - x, y0 + y, color);
        display.writePixel(x0 + x, y0 - y, color);
        display.writePixel(x0 - x, y0 - y, color);
    }
    display.endWrite();
}

// gpt-4 conversion
void WatchyOrbital::fillEllipse(int16_t x0, int16_t y0, int16_t rx, int16_t ry, uint16_t color) {
    int16_t Rx2 = rx * rx;
    int16_t Ry2 = ry * ry;
    int16_t twoRx2 = 2 * Rx2;
    int16_t twoRy2 = 2 * Ry2;
    int16_t p;
    int16_t x = 0;
    int16_t y = ry;
    int16_t px = 0;
    int16_t py = twoRx2 * y;

    display.startWrite();

    // Draw initial vertical line at center
    display.writeFastVLine(x0, y0 - y, 2 * y, color);

    // Region 1
    p = (int16_t)(Ry2 - Rx2 * ry + 0.25 * Rx2);
    while (px < py) {
        x++;
        px += twoRy2;
        if (p < 0)
            p += Ry2 + px;
        else {
            y--;
            py -= twoRx2;
            p += Ry2 + px - py;
        }
        // Draw vertical lines instead of individual pixels
        display.writeFastVLine(x0 + x, y0 - y, 2 * y, color);
        display.writeFastVLine(x0 - x, y0 - y, 2 * y, color);
    }

    // Region 2
    p = (int16_t)(Ry2 * (x + 0.5) * (x + 0.5) + Rx2 * (y - 1) * (y - 1) - Rx2 * Ry2);
    while (y > 0) {
        y--;
        py -= twoRx2;
        if (p > 0)
            p += Rx2 - py;
        else {
            x++;
            px += twoRy2;
            p += Rx2 - py + px;
        }
        // Draw vertical lines instead of individual pixels
        display.writeFastVLine(x0 + x, y0 - y, 2 * y, color);
        display.writeFastVLine(x0 - x, y0 - y, 2 * y, color);
    }

    display.endWrite();
}


void WatchyOrbital::drawDetails(float angle, unsigned int radius, String value){
    unsigned int center_x = 100;
    unsigned int center_y = 100;
    /*int16_t x1;
    int16_t y1;
    uint16_t width;
    uint16_t height;
    display.getTextBounds(value, 0, 0, &x1, &y1, &width, &height);*/
    
	// Calculate the end of the circle
    float x = center_x + radius * cos((angle - 90) * DEG2RAD);
    float y = center_y + radius * sin((angle - 90) * DEG2RAD);
    
	// Calculate a x and y delta to move the cursor based on the angle of the circle
    x = x + 3 + 7*(sin((angle + 90.0)*DEG2RAD) - 1);
    y = y + 2 + 9*(sin((180-angle)*DEG2RAD) - 0.5);
    
    display.setFont();
    display.setCursor(x, y);
    display.print(value);
}


/*
 * If Up or down are pressed, change the mode
 */
void WatchyOrbital::handleButtonPress() {
    if (guiState == WATCHFACE_STATE) {
        uint64_t wakeupBit = esp_sleep_get_ext1_wakeup_status();

        if (wakeupBit & UP_BTN_MASK) {
            mode--;
            if(mode == -1) mode = 7;
            RTC.read(currentTime);
            showWatchFace(true);
        }
        if (wakeupBit & DOWN_BTN_MASK) {
            mode++;
            if(mode == 8) mode = 0;
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
