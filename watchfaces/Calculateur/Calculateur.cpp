#include "Calculateur.h"

DS3232RTC Watchy::RTC(false); 
GxEPD2_BW<GxEPD2_154_D67, GxEPD2_154_D67::HEIGHT> Watchy::display(GxEPD2_154_D67(CS, DC, RESET, BUSY));

RTC_DATA_ATTR int guiState;
RTC_DATA_ATTR int menuIndex;
RTC_DATA_ATTR BMA423 sensor;
RTC_DATA_ATTR bool WIFI_CONFIGURED;
RTC_DATA_ATTR bool BLE_CONFIGURED;

Watchy::Watchy(){} //constructor

int decimal_minutes;

void Watchy::init(){
    esp_sleep_wakeup_cause_t wakeup_reason;
    wakeup_reason = esp_sleep_get_wakeup_cause(); //get wake up reason
    Wire.begin(SDA, SCL); //init i2c

    switch (wakeup_reason)
    {
        case ESP_SLEEP_WAKEUP_EXT0: //RTC Alarm
            RTC.alarm(ALARM_1); //resets the alarm flag in the RTC
            if(guiState == WATCHFACE_STATE){
                RTC.read(currentTime);
                calculateur();
                showWatchFace(true); //partial updates on tick
            }else{
            //
            }
            break;
        case ESP_SLEEP_WAKEUP_EXT1: //button Press
            handleButtonPress();
            break;
        default: //reset
            _rtcConfig();
            calculateur();
            RTC.alarmInterrupt(ALARM_2, false);
            RTC.alarmInterrupt(ALARM_1, true); //enable alarm interrupt
    RTC.read(currentTime);
            _bmaConfig();
            showWatchFace(false); //full update on reset
            break;
    }
    deepSleep();
}

void Watchy::deepSleep(){
  esp_sleep_enable_ext0_wakeup(RTC_PIN, 0); //enable deep sleep wake on RTC interrupt
  esp_sleep_enable_ext1_wakeup(BTN_PIN_MASK, ESP_EXT1_WAKEUP_ANY_HIGH); //enable deep sleep wake on button press
  esp_deep_sleep_start();
}

void Watchy::_rtcConfig(){
    //https://github.com/JChristensen/DS3232RTC
    RTC.squareWave(SQWAVE_NONE); //disable square wave output
    //RTC.set(compileTime()); //set RTC time to compile time
}

void Watchy::handleButtonPress(){
  uint64_t wakeupBit = esp_sleep_get_ext1_wakeup_status();
  //Menu Button
  if (wakeupBit & MENU_BTN_MASK){
    if(guiState == WATCHFACE_STATE){//enter menu state if coming from watch face
      showMenu(menuIndex, false);
    }else if(guiState == MAIN_MENU_STATE){//if already in menu, then select menu item
      switch(menuIndex)
      {
        case 0:
          showBattery();
          break;
        case 1:
          showBuzz();
          break;          
        case 2:
          showAccelerometer();
          break;
        case 3:
          setTime();
          break;
        case 4:
          setupWifi();
          break;                    
        case 5:
          showUpdateFW();
          break;
        default:
          break;                              
      }
    }else if(guiState == FW_UPDATE_STATE){
      updateFWBegin();
    }
  }
  //Back Button
  else if (wakeupBit & BACK_BTN_MASK){
    if(guiState == MAIN_MENU_STATE){//exit to watch face if already in menu
      RTC.alarm(ALARM_1); //resets the alarm flag in the RTC
      RTC.read(currentTime);
      calculateur();
      showWatchFace(false);
    }else if(guiState == APP_STATE){
      showMenu(menuIndex, false);//exit to menu if already in app
    }else if(guiState == FW_UPDATE_STATE){
      showMenu(menuIndex, false);//exit to menu if already in app
    }
  }
  //Up Button
  else if (wakeupBit & UP_BTN_MASK){
    if(guiState == MAIN_MENU_STATE){//increment menu index
      menuIndex--;
      if(menuIndex < 0){
        menuIndex = MENU_LENGTH - 1;
      }    
      showMenu(menuIndex, true);
    }
  }
  //Down Button
  else if (wakeupBit & DOWN_BTN_MASK){
    if(guiState == MAIN_MENU_STATE){//decrement menu index
      menuIndex++;
      if(menuIndex > MENU_LENGTH - 1){
        menuIndex = 0;
      }
      showMenu(menuIndex, true);
    }
  }    
}

void Watchy::showMenu(byte menuIndex, bool partialRefresh){
    display.init(0, false); //_initial_refresh to false to prevent full update on init
    display.setFullWindow();
    display.fillScreen(GxEPD_BLACK);
    display.setFont(&FreeMonoBold9pt7b);

    int16_t  x1, y1;
    uint16_t w, h;
    int16_t yPos;

    char *menuItems[] = {"Check Battery", "Vibrate Motor", "Show Accelerometer", "Set Time", "Setup WiFi", "Update Firmware"};
    for(int i=0; i<MENU_LENGTH; i++){
    yPos = 30+(MENU_HEIGHT*i);
    display.setCursor(0, yPos);
    if(i == menuIndex){
        display.getTextBounds(menuItems[i], 0, yPos, &x1, &y1, &w, &h);
        display.fillRect(x1-1, y1-10, 200, h+15, GxEPD_WHITE);
        display.setTextColor(GxEPD_BLACK);
        display.println(menuItems[i]);      
    }else{
        display.setTextColor(GxEPD_WHITE);
        display.println(menuItems[i]);
    }   
    }

    display.display(partialRefresh);
    display.hibernate();

    guiState = MAIN_MENU_STATE;    
}

void Watchy::showBattery(){
    display.init(0, false); //_initial_refresh to false to prevent full update on init
    display.setFullWindow();
    display.fillScreen(GxEPD_BLACK);
    display.setFont(&FreeMonoBold9pt7b);
    display.setTextColor(GxEPD_WHITE);
    display.setCursor(20, 30);
    display.println("Battery Voltage:");
    float voltage = getBatteryVoltage();
    display.setCursor(70, 80);
    display.print(voltage);
    display.println("V");
    display.display(false); //full refresh
    display.hibernate();

    guiState = APP_STATE;      
}

void Watchy::showBuzz(){
    display.init(0, false); //_initial_refresh to false to prevent full update on init
    display.setFullWindow();
    display.fillScreen(GxEPD_BLACK);
    display.setFont(&FreeMonoBold9pt7b);
    display.setTextColor(GxEPD_WHITE);
    display.setCursor(70, 80);
    display.println("Buzz!");
    display.display(false); //full refresh
    display.hibernate();
    vibMotor();
    showMenu(menuIndex, false);    
}

void Watchy::vibMotor(uint8_t intervalMs, uint8_t length){
    pinMode(VIB_MOTOR_PIN, OUTPUT);
    bool motorOn = false;
    for(int i=0; i<length; i++){
        motorOn = !motorOn;
        digitalWrite(VIB_MOTOR_PIN, motorOn);
        delay(intervalMs);
    }
}

void Watchy::setTime(){

    guiState = APP_STATE;

    RTC.read(currentTime);

    int8_t minute = currentTime.Minute;
    int8_t hour = currentTime.Hour;
    int8_t day = currentTime.Day;
    int8_t month = currentTime.Month;
    int8_t year = currentTime.Year + YEAR_OFFSET - 2000;

    int8_t setIndex = SET_HOUR;

    int16_t  x1, y1;
    uint16_t w, h;
    int8_t blink = 0;

    pinMode(DOWN_BTN_PIN, INPUT);
    pinMode(UP_BTN_PIN, INPUT);
    pinMode(MENU_BTN_PIN, INPUT);  
    pinMode(BACK_BTN_PIN, INPUT);  

    display.init(0, true); //_initial_refresh to false to prevent full update on init
    display.setFullWindow();

    while(1){

    if(digitalRead(MENU_BTN_PIN) == 1){
        setIndex++;
        if(setIndex > SET_DAY){
        break;
        }
    }
    if(digitalRead(BACK_BTN_PIN) == 1){
        if(setIndex != SET_HOUR){
        setIndex--;
        }
    }      

    blink = 1 - blink;

    if(digitalRead(DOWN_BTN_PIN) == 1){
        blink = 1;
        switch(setIndex){
        case SET_HOUR:
            hour == 23 ? (hour = 0) : hour++;
            break;
        case SET_MINUTE:
            minute == 59 ? (minute = 0) : minute++;
            break;
        case SET_YEAR:
            year == 99 ? (year = 20) : year++;
            break;
        case SET_MONTH:
            month == 12 ? (month = 1) : month++;
            break;
        case SET_DAY:
            day == 31 ? (day = 1) : day++;
            break;                         
        default:
            break;
        }      
    }

    if(digitalRead(UP_BTN_PIN) == 1){
        blink = 1;
        switch(setIndex){
        case SET_HOUR:
            hour == 0 ? (hour = 23) : hour--;
            break;
        case SET_MINUTE:
            minute == 0 ? (minute = 59) : minute--;
            break;
        case SET_YEAR:
            year == 20 ? (year = 99) : year--;
            break;
        case SET_MONTH:
            month == 1 ? (month = 12) : month--;
            break;
        case SET_DAY:
            day == 1 ? (day = 31) : day--;
            break;          
        default:
            break;
        }   
    }    

    display.fillScreen(GxEPD_BLACK);
    display.setTextColor(GxEPD_WHITE);
    display.setFont(&DSEG7_Classic_Bold_53);

    display.setCursor(5, 80);
    if(setIndex == SET_HOUR){//blink hour digits
        display.setTextColor(blink ? GxEPD_WHITE : GxEPD_BLACK);
    }
    if(hour < 10){
        display.print("0");      
    }
    display.print(hour);

    display.setTextColor(GxEPD_WHITE);
    display.print(":");

    display.setCursor(108, 80);
    if(setIndex == SET_MINUTE){//blink minute digits
        display.setTextColor(blink ? GxEPD_WHITE : GxEPD_BLACK);
    }
    if(minute < 10){
        display.print("0");      
    }
    display.print(minute);

    display.setTextColor(GxEPD_WHITE);

    display.setFont(&FreeMonoBold9pt7b);
    display.setCursor(45, 150);
    if(setIndex == SET_YEAR){//blink minute digits
        display.setTextColor(blink ? GxEPD_WHITE : GxEPD_BLACK);
    }    
    display.print(2000+year);

    display.setTextColor(GxEPD_WHITE);
    display.print("/");

    if(setIndex == SET_MONTH){//blink minute digits
        display.setTextColor(blink ? GxEPD_WHITE : GxEPD_BLACK);
    }   
    if(month < 10){
        display.print("0");      
    }     
    display.print(month);

    display.setTextColor(GxEPD_WHITE);
    display.print("/");

    if(setIndex == SET_DAY){//blink minute digits
        display.setTextColor(blink ? GxEPD_WHITE : GxEPD_BLACK);
    }       
    if(day < 10){
        display.print("0");      
    }     
    display.print(day); 
    display.display(true); //partial refresh
    }

    display.hibernate();

    const time_t FUDGE(10);//fudge factor to allow for upload time, etc. (seconds, YMMV)
    tmElements_t tm;
    tm.Month = month;
    tm.Day = day;
    tm.Year = year + 2000 - YEAR_OFFSET;//offset from 1970, since year is stored in uint8_t
    tm.Hour = hour;
    tm.Minute = minute;
    tm.Second = 0;

    time_t t = makeTime(tm) + FUDGE;
    RTC.set(t);

    showMenu(menuIndex, false);

}

void Watchy::showAccelerometer(){
    display.init(0, true); //_initial_refresh to false to prevent full update on init
    display.setFullWindow();
    display.fillScreen(GxEPD_BLACK);
    display.setFont(&FreeMonoBold9pt7b);
    display.setTextColor(GxEPD_WHITE);

    Accel acc;

    long previousMillis = 0;
    long interval = 200;  

    guiState = APP_STATE;

    pinMode(BACK_BTN_PIN, INPUT);

    while(1){

    unsigned long currentMillis = millis();

    if(digitalRead(BACK_BTN_PIN) == 1){
        break;
    }

    if(currentMillis - previousMillis > interval){
        previousMillis = currentMillis;
        // Get acceleration data
        bool res = sensor.getAccel(acc);
        uint8_t direction = sensor.getDirection();
        display.setFullWindow();
        display.fillScreen(GxEPD_BLACK);      
        display.setCursor(0, 30);
        if(res == false) {
            display.println("getAccel FAIL");
        }else{
        display.print("  X:"); display.println(acc.x);
        display.print("  Y:"); display.println(acc.y);
        display.print("  Z:"); display.println(acc.z);

        display.setCursor(30, 130);
        switch(direction){
            case DIRECTION_DISP_DOWN:
                display.println("FACE DOWN");
                break;
            case DIRECTION_DISP_UP:
                display.println("FACE UP");
                break;
            case DIRECTION_BOTTOM_EDGE:
                display.println("BOTTOM EDGE");
                break;
            case DIRECTION_TOP_EDGE:
                display.println("TOP EDGE");
                break;
            case DIRECTION_RIGHT_EDGE:
                display.println("RIGHT EDGE");
                break;
            case DIRECTION_LEFT_EDGE:
                display.println("LEFT EDGE");
                break;
            default:
                display.println("ERROR!!!");
                break;
        }

        }
        display.display(true); //full refresh
    }
    }

    showMenu(menuIndex, false);
}

void Watchy::showWatchFace(bool partialRefresh){
  display.init(0, false); //_initial_refresh to false to prevent full update on init
  display.setFullWindow();
  drawWatchFace();
  display.display(partialRefresh); //partial refresh
  display.hibernate();
  guiState = WATCHFACE_STATE;
}

void Watchy::drawWatchFace(){
  int x = 100;
  int y = 125;
    
  int16_t x1, y1;
  uint16_t w, h;

  String displayTime;

  if (decimal_minutes == 0) {
    displayTime = "NEW";
  } else {
    displayTime = String(1000 - decimal_minutes);
  }  

  // center time on dsiplay
  display.setFont(&MADE_Sunflower_PERSONAL_USE39pt7b);
  display.setTextWrap(false);
  display.getTextBounds(displayTime, x, y, &x1, &y1, &w, &h);
  display.setCursor(x - w / 2, y);
  display.print(displayTime); 
}

weatherData Watchy::getWeatherData(){

    weatherData currentWeather;

    if(connectWiFi()){//Use Weather API for live data if WiFi is connected
        HTTPClient http;
        http.setConnectTimeout(3000);//3 second max timeout
        String weatherQueryURL = String(OPENWEATHERMAP_URL) + String(CITY_NAME) + String(",") + String(COUNTRY_CODE) + String("&units=") + String(TEMP_UNIT) + String("&appid=") + String(OPENWEATHERMAP_APIKEY);
        http.begin(weatherQueryURL.c_str());
        int httpResponseCode = http.GET();
        if(httpResponseCode == 200) {
            String payload = http.getString();
            JSONVar responseObject = JSON.parse(payload);
            currentWeather.temperature = int(responseObject["main"]["temp"]);
            currentWeather.weatherConditionCode = int(responseObject["weather"][0]["id"]);            
        }else{
            //http error
        }
        http.end();
        //turn off radios
        WiFi.mode(WIFI_OFF);
        btStop();
    }else{//No WiFi, use RTC Temperature
        uint8_t temperature = RTC.temperature() / 4; //celsius
        if(TEMP_UNIT == "imperial"){
            temperature = temperature * 9. / 5. + 32.; //fahrenheit
        }
        currentWeather.temperature = temperature;
        currentWeather.weatherConditionCode = 800;
    } 
    return currentWeather;
}

float Watchy::getBatteryVoltage(){
    return analogRead(ADC_PIN) / 4096.0 * 7.23;
}

uint16_t Watchy::_readRegister(uint8_t address, uint8_t reg, uint8_t *data, uint16_t len)
{
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.endTransmission();
    Wire.requestFrom((uint8_t)address, (uint8_t)len);
    uint8_t i = 0;
    while (Wire.available()) {
        data[i++] = Wire.read();
    }
    return 0;
}

uint16_t Watchy::_writeRegister(uint8_t address, uint8_t reg, uint8_t *data, uint16_t len)
{
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(data, len);
    return (0 !=  Wire.endTransmission());
}

void Watchy::_bmaConfig(){
 
    if (sensor.begin(_readRegister, _writeRegister, delay) == false) {
        //fail to init BMA
        return;
    }

    // Accel parameter structure
    Acfg cfg;
    /*!
        Output data rate in Hz, Optional parameters:
            - BMA4_OUTPUT_DATA_RATE_0_78HZ
            - BMA4_OUTPUT_DATA_RATE_1_56HZ
            - BMA4_OUTPUT_DATA_RATE_3_12HZ
            - BMA4_OUTPUT_DATA_RATE_6_25HZ
            - BMA4_OUTPUT_DATA_RATE_12_5HZ
            - BMA4_OUTPUT_DATA_RATE_25HZ
            - BMA4_OUTPUT_DATA_RATE_50HZ
            - BMA4_OUTPUT_DATA_RATE_100HZ
            - BMA4_OUTPUT_DATA_RATE_200HZ
            - BMA4_OUTPUT_DATA_RATE_400HZ
            - BMA4_OUTPUT_DATA_RATE_800HZ
            - BMA4_OUTPUT_DATA_RATE_1600HZ
    */
    cfg.odr = BMA4_OUTPUT_DATA_RATE_100HZ;
    /*!
        G-range, Optional parameters:
            - BMA4_ACCEL_RANGE_2G
            - BMA4_ACCEL_RANGE_4G
            - BMA4_ACCEL_RANGE_8G
            - BMA4_ACCEL_RANGE_16G
    */
    cfg.range = BMA4_ACCEL_RANGE_2G;
    /*!
        Bandwidth parameter, determines filter configuration, Optional parameters:
            - BMA4_ACCEL_OSR4_AVG1
            - BMA4_ACCEL_OSR2_AVG2
            - BMA4_ACCEL_NORMAL_AVG4
            - BMA4_ACCEL_CIC_AVG8
            - BMA4_ACCEL_RES_AVG16
            - BMA4_ACCEL_RES_AVG32
            - BMA4_ACCEL_RES_AVG64
            - BMA4_ACCEL_RES_AVG128
    */
    cfg.bandwidth = BMA4_ACCEL_NORMAL_AVG4;

    /*! Filter performance mode , Optional parameters:
        - BMA4_CIC_AVG_MODE
        - BMA4_CONTINUOUS_MODE
    */
    cfg.perf_mode = BMA4_CONTINUOUS_MODE;

    // Configure the BMA423 accelerometer
    sensor.setAccelConfig(cfg);

    // Enable BMA423 accelerometer
    // Warning : Need to use feature, you must first enable the accelerometer
    // Warning : Need to use feature, you must first enable the accelerometer
    sensor.enableAccel();

    struct bma4_int_pin_config config ;
    config.edge_ctrl = BMA4_LEVEL_TRIGGER;
    config.lvl = BMA4_ACTIVE_HIGH;
    config.od = BMA4_PUSH_PULL;
    config.output_en = BMA4_OUTPUT_ENABLE;
    config.input_en = BMA4_INPUT_DISABLE;
    // The correct trigger interrupt needs to be configured as needed
    sensor.setINTPinConfig(config, BMA4_INTR1_MAP);

    struct bma423_axes_remap remap_data;
    remap_data.x_axis = 1;
    remap_data.x_axis_sign = 0;
    remap_data.y_axis = 0;
    remap_data.y_axis_sign = 0;
    remap_data.z_axis  = 2;
    remap_data.z_axis_sign  = 0;
    // Need to raise the wrist function, need to set the correct axis
    sensor.setRemapAxes(&remap_data);

    // Enable BMA423 isStepCounter feature
    sensor.enableFeature(BMA423_STEP_CNTR, true);
    // Enable BMA423 isTilt feature
    sensor.enableFeature(BMA423_TILT, true);
    // Enable BMA423 isDoubleClick feature
    sensor.enableFeature(BMA423_WAKEUP, true);

    // Reset steps
    sensor.resetStepCounter();

    // Turn on feature interrupt
    sensor.enableStepCountInterrupt();
    sensor.enableTiltInterrupt();
    // It corresponds to isDoubleClick interrupt
    sensor.enableWakeupInterrupt();  
}

void Watchy::setupWifi(){
  WiFiManager wifiManager;
  wifiManager.resetSettings();
  wifiManager.setTimeout(WIFI_AP_TIMEOUT);
  wifiManager.setAPCallback(_configModeCallback);
  if(!wifiManager.autoConnect(WIFI_AP_SSID)) {//WiFi setup failed
    display.init(0, false); //_initial_refresh to false to prevent full update on init
    display.setFullWindow();
    display.fillScreen(GxEPD_BLACK);
    display.setFont(&FreeMonoBold9pt7b);
    display.setTextColor(GxEPD_WHITE);
    display.setCursor(0, 30);
    display.println("Setup failed &");
    display.println("timed out!");
    display.display(false); //full refresh
    display.hibernate();
  }else{
    display.init(0, false);//_initial_refresh to false to prevent full update on init
    display.setFullWindow();
    display.fillScreen(GxEPD_BLACK);
    display.setFont(&FreeMonoBold9pt7b);
    display.setTextColor(GxEPD_WHITE);
    display.println("Connected to");
    display.println(WiFi.SSID());
    display.display(false);//full refresh
    display.hibernate();
  }
  //turn off radios
  WiFi.mode(WIFI_OFF);
  btStop();

  guiState = APP_STATE;  
}

void Watchy::_configModeCallback (WiFiManager *myWiFiManager) {
  display.init(0, false); //_initial_refresh to false to prevent full update on init
  display.setFullWindow();
  display.fillScreen(GxEPD_BLACK);
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(GxEPD_WHITE);
  display.setCursor(0, 30);
  display.println("Connect to");
  display.print("SSID: ");
  display.println(WIFI_AP_SSID);
  display.print("IP: ");
  display.println(WiFi.softAPIP());
  display.display(false); //full refresh
  display.hibernate();
}

bool Watchy::connectWiFi(){
    if(WL_CONNECT_FAILED == WiFi.begin()){//WiFi not setup, you can also use hard coded credentials with WiFi.begin(SSID,PASS);
        WIFI_CONFIGURED = false;
    }else{
        if(WL_CONNECTED == WiFi.waitForConnectResult()){//attempt to connect for 10s
            WIFI_CONFIGURED = true;
        }else{//connection failed, time out
            WIFI_CONFIGURED = false;
            //turn off radios
            WiFi.mode(WIFI_OFF);
            btStop();
        }
    }
    return WIFI_CONFIGURED;
}

void Watchy::showUpdateFW(){
    display.init(0, false); //_initial_refresh to false to prevent full update on init
    display.setFullWindow();
    display.fillScreen(GxEPD_BLACK);
    display.setFont(&FreeMonoBold9pt7b);
    display.setTextColor(GxEPD_WHITE);
    display.setCursor(0, 30);
    display.println("Please Visit");
    display.println("watchy.sqfmi.com");
    display.println("with a Bluetooth");
    display.println("enabled device");
    display.println(" ");
    display.println("Press menu button");
    display.println("again when ready");
    display.println(" ");
    display.println("Keep USB powered");
    display.display(false); //full refresh
    display.hibernate();

    guiState = FW_UPDATE_STATE;  
}

void Watchy::updateFWBegin(){
    display.init(0, false); //_initial_refresh to false to prevent full update on init
    display.setFullWindow();
    display.fillScreen(GxEPD_BLACK);
    display.setFont(&FreeMonoBold9pt7b);
    display.setTextColor(GxEPD_WHITE);
    display.setCursor(0, 30);
    display.println("Bluetooth Started");
    display.println(" ");
    display.println("Watchy BLE OTA");
    display.println(" ");
    display.println("Waiting for");
    display.println("connection...");
    display.display(false); //full refresh

    BLE BT;
    BT.begin("Watchy BLE OTA");
    int prevStatus = -1;
    int currentStatus;

    while(1){
    currentStatus = BT.updateStatus();
    if(prevStatus != currentStatus || prevStatus == 1){
        if(currentStatus == 0){
        display.setFullWindow();
        display.fillScreen(GxEPD_BLACK);
        display.setFont(&FreeMonoBold9pt7b);
        display.setTextColor(GxEPD_WHITE);
        display.setCursor(0, 30);
        display.println("BLE Connected!");
        display.println(" ");
        display.println("Waiting for");
        display.println("upload...");
        display.display(false); //full refresh
        }
        if(currentStatus == 1){
        display.setFullWindow();
        display.fillScreen(GxEPD_BLACK);
        display.setFont(&FreeMonoBold9pt7b);
        display.setTextColor(GxEPD_WHITE);
        display.setCursor(0, 30);
        display.println("Downloading");
        display.println("firmware:");
        display.println(" ");
        display.print(BT.howManyBytes());
        display.println(" bytes");
        display.display(true); //partial refresh        
        }
        if(currentStatus == 2){
        display.setFullWindow();
        display.fillScreen(GxEPD_BLACK);
        display.setFont(&FreeMonoBold9pt7b);
        display.setTextColor(GxEPD_WHITE);
        display.setCursor(0, 30);
        display.println("Download");
        display.println("completed!");
        display.println(" ");
        display.println("Rebooting...");
        display.display(false); //full refresh

        delay(2000);
        esp_restart();           
        }
        if(currentStatus == 4){
        display.setFullWindow();
        display.fillScreen(GxEPD_BLACK);
        display.setFont(&FreeMonoBold9pt7b);
        display.setTextColor(GxEPD_WHITE);
        display.setCursor(0, 30);
        display.println("BLE Disconnected!");
        display.println(" ");
        display.println("exiting...");
        display.display(false); //full refresh
        delay(1000);
        break;
        }
        prevStatus = currentStatus;
    }
    delay(100);
    }

    //turn off radios
    WiFi.mode(WIFI_OFF);
    btStop();
    showMenu(menuIndex, false);
}

// calculate decimal time and set new alarm
void Watchy::calculateur(){
  RTC.read(currentTime);
  // rounded decimal minute steps in sexagesimal seconds -> no millis available from DS3232RTC
  int steps[] = {86, 173, 259, 346, 432, 518, 605, 691, 778, 864, 950, 1037, 1123, 1210, 1296, 1382, 1469, 1555, 1642, 1728, 1814, 1901, 1987, 2074, 2160, 2246, 2333, 2419, 2506, 2592, 2678, 2765, 2851, 2938, 3024, 3110, 3197, 3283, 3370, 3456, 3542, 3629, 3715, 3802, 3888, 3974, 4061, 4147, 4234, 4320, 4406, 4493, 4579, 4666, 4752, 4838, 4925, 5011, 5098, 5184, 5270, 5357, 5443, 5530, 5616, 5702, 5789, 5875, 5962, 6048, 6134, 6221, 6307, 6394, 6480, 6566, 6653, 6739, 6826, 6912, 6998, 7085, 7171, 7258, 7344, 7430, 7517, 7603, 7690, 7776, 7862, 7949, 8035, 8122, 8208, 8294, 8381, 8467, 8554, 8640, 8726, 8813, 8899, 8986, 9072, 9158, 9245, 9331, 9418, 9504, 9590, 9677, 9763, 9850, 9936, 10022, 10109, 10195, 10282, 10368, 10454, 10541, 10627, 10714, 10800, 10886, 10973, 11059, 11146, 11232, 11318, 11405, 11491, 11578, 11664, 11750, 11837, 11923, 12010, 12096, 12182, 12269, 12355, 12442, 12528, 12614, 12701, 12787, 12874, 12960, 13046, 13133, 13219, 13306, 13392, 13478, 13565, 13651, 13738, 13824, 13910, 13997, 14083, 14170, 14256, 14342, 14429, 14515, 14602, 14688, 14774, 14861, 14947, 15034, 15120, 15206, 15293, 15379, 15466, 15552, 15638, 15725, 15811, 15898, 15984, 16070, 16157, 16243, 16330, 16416, 16502, 16589, 16675, 16762, 16848, 16934, 17021, 17107, 17194, 17280, 17366, 17453, 17539, 17626, 17712, 17798, 17885, 17971, 18058, 18144, 18230, 18317, 18403, 18490, 18576, 18662, 18749, 18835, 18922, 19008, 19094, 19181, 19267, 19354, 19440, 19526, 19613, 19699, 19786, 19872, 19958, 20045, 20131, 20218, 20304, 20390, 20477, 20563, 20650, 20736, 20822, 20909, 20995, 21082, 21168, 21254, 21341, 21427, 21514, 21600, 21686, 21773, 21859, 21946, 22032, 22118, 22205, 22291, 22378, 22464, 22550, 22637, 22723, 22810, 22896, 22982, 23069, 23155, 23242, 23328, 23414, 23501, 23587, 23674, 23760, 23846, 23933, 24019, 24106, 24192, 24278, 24365, 24451, 24538, 24624, 24710, 24797, 24883, 24970, 25056, 25142, 25229, 25315, 25402, 25488, 25574, 25661, 25747, 25834, 25920, 26006, 26093, 26179, 26266, 26352, 26438, 26525, 26611, 26698, 26784, 26870, 26957, 27043, 27130, 27216, 27302, 27389, 27475, 27562, 27648, 27734, 27821, 27907, 27994, 28080, 28166, 28253, 28339, 28426, 28512, 28598, 28685, 28771, 28858, 28944, 29030, 29117, 29203, 29290, 29376, 29462, 29549, 29635, 29722, 29808, 29894, 29981, 30067, 30154, 30240, 30326, 30413, 30499, 30586, 30672, 30758, 30845, 30931, 31018, 31104, 31190, 31277, 31363, 31450, 31536, 31622, 31709, 31795, 31882, 31968, 32054, 32141, 32227, 32314, 32400, 32486, 32573, 32659, 32746, 32832, 32918, 33005, 33091, 33178, 33264, 33350, 33437, 33523, 33610, 33696, 33782, 33869, 33955, 34042, 34128, 34214, 34301, 34387, 34474, 34560, 34646, 34733, 34819, 34906, 34992, 35078, 35165, 35251, 35338, 35424, 35510, 35597, 35683, 35770, 35856, 35942, 36029, 36115, 36202, 36288, 36374, 36461, 36547, 36634, 36720, 36806, 36893, 36979, 37066, 37152, 37238, 37325, 37411, 37498, 37584, 37670, 37757, 37843, 37930, 38016, 38102, 38189, 38275, 38362, 38448, 38534, 38621, 38707, 38794, 38880, 38966, 39053, 39139, 39226, 39312, 39398, 39485, 39571, 39658, 39744, 39830, 39917, 40003, 40090, 40176, 40262, 40349, 40435, 40522, 40608, 40694, 40781, 40867, 40954, 41040, 41126, 41213, 41299, 41386, 41472, 41558, 41645, 41731, 41818, 41904, 41990, 42077, 42163, 42250, 42336, 42422, 42509, 42595, 42682, 42768, 42854, 42941, 43027, 43114, 43200, 43286, 43373, 43459, 43546, 43632, 43718, 43805, 43891, 43978, 44064, 44150, 44237, 44323, 44410, 44496, 44582, 44669, 44755, 44842, 44928, 45014, 45101, 45187, 45274, 45360, 45446, 45533, 45619, 45706, 45792, 45878, 45965, 46051, 46138, 46224, 46310, 46397, 46483, 46570, 46656, 46742, 46829, 46915, 47002, 47088, 47174, 47261, 47347, 47434, 47520, 47606, 47693, 47779, 47866, 47952, 48038, 48125, 48211, 48298, 48384, 48470, 48557, 48643, 48730, 48816, 48902, 48989, 49075, 49162, 49248, 49334, 49421, 49507, 49594, 49680, 49766, 49853, 49939, 50026, 50112, 50198, 50285, 50371, 50458, 50544, 50630, 50717, 50803, 50890, 50976, 51062, 51149, 51235, 51322, 51408, 51494, 51581, 51667, 51754, 51840, 51926, 52013, 52099, 52186, 52272, 52358, 52445, 52531, 52618, 52704, 52790, 52877, 52963, 53050, 53136, 53222, 53309, 53395, 53482, 53568, 53654, 53741, 53827, 53914, 54000, 54086, 54173, 54259, 54346, 54432, 54518, 54605, 54691, 54778, 54864, 54950, 55037, 55123, 55210, 55296, 55382, 55469, 55555, 55642, 55728, 55814, 55901, 55987, 56074, 56160, 56246, 56333, 56419, 56506, 56592, 56678, 56765, 56851, 56938, 57024, 57110, 57197, 57283, 57370, 57456, 57542, 57629, 57715, 57802, 57888, 57974, 58061, 58147, 58234, 58320, 58406, 58493, 58579, 58666, 58752, 58838, 58925, 59011, 59098, 59184, 59270, 59357, 59443, 59530, 59616, 59702, 59789, 59875, 59962, 60048, 60134, 60221, 60307, 60394, 60480, 60566, 60653, 60739, 60826, 60912, 60998, 61085, 61171, 61258, 61344, 61430, 61517, 61603, 61690, 61776, 61862, 61949, 62035, 62122, 62208, 62294, 62381, 62467, 62554, 62640, 62726, 62813, 62899, 62986, 63072, 63158, 63245, 63331, 63418, 63504, 63590, 63677, 63763, 63850, 63936, 64022, 64109, 64195, 64282, 64368, 64454, 64541, 64627, 64714, 64800, 64886, 64973, 65059, 65146, 65232, 65318, 65405, 65491, 65578, 65664, 65750, 65837, 65923, 66010, 66096, 66182, 66269, 66355, 66442, 66528, 66614, 66701, 66787, 66874, 66960, 67046, 67133, 67219, 67306, 67392, 67478, 67565, 67651, 67738, 67824, 67910, 67997, 68083, 68170, 68256, 68342, 68429, 68515, 68602, 68688, 68774, 68861, 68947, 69034, 69120, 69206, 69293, 69379, 69466, 69552, 69638, 69725, 69811, 69898, 69984, 70070, 70157, 70243, 70330, 70416, 70502, 70589, 70675, 70762, 70848, 70934, 71021, 71107, 71194, 71280, 71366, 71453, 71539, 71626, 71712, 71798, 71885, 71971, 72058, 72144, 72230, 72317, 72403, 72490, 72576, 72662, 72749, 72835, 72922, 73008, 73094, 73181, 73267, 73354, 73440, 73526, 73613, 73699, 73786, 73872, 73958, 74045, 74131, 74218, 74304, 74390, 74477, 74563, 74650, 74736, 74822, 74909, 74995, 75082, 75168, 75254, 75341, 75427, 75514, 75600, 75686, 75773, 75859, 75946, 76032, 76118, 76205, 76291, 76378, 76464, 76550, 76637, 76723, 76810, 76896, 76982, 77069, 77155, 77242, 77328, 77414, 77501, 77587, 77674, 77760, 77846, 77933, 78019, 78106, 78192, 78278, 78365, 78451, 78538, 78624, 78710, 78797, 78883, 78970, 79056, 79142, 79229, 79315, 79402, 79488, 79574, 79661, 79747, 79834, 79920, 80006, 80093, 80179, 80266, 80352, 80438, 80525, 80611, 80698, 80784, 80870, 80957, 81043, 81130, 81216, 81302, 81389, 81475, 81562, 81648, 81734, 81821, 81907, 81994, 82080, 82166, 82253, 82339, 82426, 82512, 82598, 82685, 82771, 82858, 82944, 83030, 83117, 83203, 83290, 83376, 83462, 83549, 83635, 83722, 83808, 83894, 83981, 84067, 84154, 84240, 84326, 84413, 84499, 84586, 84672, 84758, 84845, 84931, 85018, 85104, 85190, 85277, 85363, 85450, 85536, 85622, 85709, 85795, 85882, 85968, 86054, 86141, 86227, 86314, 86400};
  
  // convert current time to seconds
  int secs_total = (currentTime.Hour * 3600) + (currentTime.Minute * 60) + currentTime.Second;
  int next_step;

  for (int x = 0; x < 1000; x++) {
        if (secs_total < steps[x]) {
            next_step = steps[x];
            decimal_minutes = x;
            break;
        }
    }

  int next_wake_m = floor(next_step / 60 % 60);
  int next_wake_s = floor(next_step % 60);

  RTC.setAlarm(ALM1_MATCH_MINUTES, next_wake_s, next_wake_m, 0, 0);
}

// time_t compileTime()
// {   
//     const time_t FUDGE(10);    //fudge factor to allow for upload time, etc. (seconds, YMMV)
//     const char *compDate = __DATE__, *compTime = __TIME__, *months = "JanFebMarAprMayJunJulAugSepOctNovDec";
//     char compMon[3], *m;

//     strncpy(compMon, compDate, 3);
//     compMon[3] = '\0';
//     m = strstr(months, compMon);

//     tmElements_t tm;
//     tm.Month = ((m - months) / 3 + 1);
//     tm.Day = atoi(compDate + 4);
//     tm.Year = atoi(compDate + 7) - YEAR_OFFSET; // offset from 1970, since year is stored in uint8_t
//     tm.Hour = atoi(compTime);
//     tm.Minute = atoi(compTime + 3);
//     tm.Second = atoi(compTime + 6);

//     time_t t = makeTime(tm);
//     return t + FUDGE;        //add fudge factor to allow for compile time
// }
