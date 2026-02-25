inline void drawCaptnWednesday(Watchy &watchy, float batt) {

    const uint16_t fg = UiSDK::getWatchfaceFg(BASE_POLARITY);
    const uint16_t bg = UiSDK::getWatchfaceBg(BASE_POLARITY);
    (void)batt;

    int16_t x1, y1;
    uint16_t w, h;
    String textstring, textstringday, textstringtime, textstringsteps;

    UiSDK::initScreen(watchy.display);
    watchy.display.fillScreen(bg);
    watchy.display.drawBitmap(0,0, captnwednesday_img, 200, 200, fg);

    watchy.display.setTextColor(fg);
    watchy.display.setTextWrap(false);

    watchy.display.setFont(&Tintin_Dialogue9pt7b);

    textstringday = dayStr(watchy.currentTime.Wday);
    watchy.display.getTextBounds(textstring, 0, 0, &x1, &y1, &w, &h);
    watchy.display.setCursor(76, 66);
    watchy.display.print(textstringday);
    
    watchy.display.setFont(&Tintin_Dialogue10pt7b);
    
    textstring = watchy.currentTime.Day;
    textstring += "-";
    textstring += monthShortStr(watchy.currentTime.Month);

    watchy.display.getTextBounds(textstring, 0, 0, &x1, &y1, &w, &h);
    watchy.display.setCursor(0, 194);
    watchy.display.print(textstring);
    
    watchy.display.setFont(&Tintin_Dialogue16pt7b);
    watchy.display.setCursor(60, 199);
    int displayHour;
    if(HOUR_12_24==12){
      displayHour = ((watchy.currentTime.Hour+11)%12)+1;
    } else {
      displayHour = watchy.currentTime.Hour;
    }
    if(displayHour < 10){
        watchy.display.print("0");
    }
    watchy.display.print(displayHour);
    watchy.display.print(":");
    if(watchy.currentTime.Minute < 10){
        watchy.display.print("0");
    }
    watchy.display.println(watchy.currentTime.Minute);

    int8_t batteryLevel = 0;
    float VBAT = watchy.getBatteryVoltage();
    if(VBAT > 4.1){
        batteryLevel = 3;
    }
    else if(VBAT > 3.95 && VBAT <= 4.1){
        batteryLevel = 2;
    }
    else if(VBAT > 3.80 && VBAT <= 3.95){
        batteryLevel = 1;
    }
    else if(VBAT <= 3.80){
        batteryLevel = 0;
    }

    for(int8_t batterySegments = 0; batterySegments < batteryLevel; batterySegments++){
        watchy.display.fillRect(159, 151 + (batterySegments * (BATTERY_SEGMENT_SPACING-2)), BATTERY_SEGMENT_HEIGHT-3, BATTERY_SEGMENT_WIDTH-2, fg);
    }

    if (watchy.currentTime.Hour == 0 && watchy.currentTime.Minute == 0){
      sensor.resetStepCounter();
    }
    uint32_t stepCount = sensor.getCounter();
    watchy.display.setFont(&Tintin_Dialogue8pt7b);
    watchy.display.setCursor(160, 184);
    watchy.display.print("Steps");
    watchy.display.drawFastHLine(159,184, 40, fg);
    
    watchy.display.setFont(&Tintin_Dialogue8pt7b);
    textstringsteps = String(stepCount);
    watchy.display.getTextBounds(textstringsteps, 0, 0, &x1, &y1, &w, &h);
    watchy.display.setCursor(198 - w, 199);
    watchy.display.println(textstringsteps);
    
}
