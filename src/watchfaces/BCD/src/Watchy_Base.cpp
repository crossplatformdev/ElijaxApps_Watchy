#include "Watchy_Base.h"
#include "sdk/UiSDK.h"



/*
 * DEFINITIONS
 */

#define FONT_SMALL       FreeSans9pt7b


// Store in RTC RAM, otherwise we loose information between different interrupts
static RTC_DATA_ATTR uint8_t bcd_rotation = 0;
RTC_DATA_ATTR int16_t bcd_alarm_timer = -1;
RTC_DATA_ATTR bool bcd_dark_mode = false;

// Variables needed to show data from our MQTT broker
RTC_DATA_ATTR bool bcd_show_mqqt_data = false;

#if WATCHY_BCD_HAS_PUBSUBCLIENT
static RTC_DATA_ATTR volatile int bcd_received_mqtt_data = false;

#define MQTT_NUM_DATA               6
#define MQTT_RECEIVED_ALL_DATA      (bcd_received_mqtt_data >= MQTT_NUM_DATA)
#define MQTT_CLEAR                  bcd_received_mqtt_data=0
#define MQTT_RECEIVED_DATA          bcd_received_mqtt_data++

static RTC_DATA_ATTR float bcd_indoor_temp = 0.0;
static RTC_DATA_ATTR int bcd_indoor_co2 = 0.0;
static RTC_DATA_ATTR float bcd_outdoor_temp = 0.0;
static RTC_DATA_ATTR int bcd_outdoor_rain = 0.0;
static RTC_DATA_ATTR int bcd_outdoor_wind = 0;
static RTC_DATA_ATTR int bcd_outdoor_gusts = 0;
#endif

static RTC_DATA_ATTR bool bcd_sleep_mode = false;

/*
 * FUNCTIONS
 */
BCDWatchyBase::BCDWatchyBase(){

}


void BCDWatchyBase::init(esp_sleep_wakeup_cause_t wakeup_reason){

    switch (wakeup_reason)
    {
        case ESP_SLEEP_WAKEUP_EXT0: //RTC Alarm

            // Handle alarm
            if(bcd_alarm_timer == 0){
                vibrate(3, 500);
                bcd_alarm_timer = -1;

                // Continue to update watch face
            }

            // Handle classical tick
            RTC.clearAlarm();

            // Only for visualization and to ensure that alarm is not triggered
            // again and a gain as the alarm flag is internally set every time...
            if(bcd_alarm_timer > 0){
                bcd_alarm_timer--;
            }

            if(guiState == WATCHFACE_STATE && !bcd_show_mqqt_data){
                RTC.read(currentTime);

                if(currentTime.Hour == SLEEP_HOUR && currentTime.Minute == SLEEP_MINUTE){
                    bcd_sleep_mode = true;
                }

                showWatchFace(true); //partial updates on tick
            }
            break;

        case ESP_SLEEP_WAKEUP_EXT1: //button Press + no handling if wakeup
            if(bcd_sleep_mode){
                bcd_sleep_mode = false;
                RTC.clearAlarm();

                RTC.read(currentTime);
                showWatchFace(false); //full update on wakeup from sleep mode
                break;
            }

            handleButtonPress();
            break;

        default: //reset
            _rtcConfig();
            showWatchFace(false); //full update on reset
            break;
    }
}


bool BCDWatchyBase::watchFaceDisabled(){
    return bcd_show_mqqt_data || bcd_sleep_mode;
}


void BCDWatchyBase::handleButtonPress(){
    uint64_t wakeupBit = esp_sleep_get_ext1_wakeup_status();

    if (IS_DOUBLE_TAP){
        // Interrupt handling managed by core Watchy

        // To be defined in the watch face what we want exactly
        // to do. Therefore, no return;
    }

    if (IS_BTN_LEFT_UP){

        RTC.read(currentTime);

        if(bcd_alarm_timer < 0){
            bcd_alarm_timer = 0;
        }

        if(bcd_alarm_timer < 60 * 24){
            bcd_alarm_timer += bcd_alarm_timer < 60 ? 5 : 10;
        }

        // Sum minutes to current time
        // Upstream used DS3232RTC alarms; this firmware uses Watchy32KRTC.
        // Keep a simple minute countdown and trigger when it reaches 0.
        vibrate();

        showWatchFace(true);
        return;
    }

    if(IS_BTN_RIGHT_UP){
        bcd_show_mqqt_data = bcd_show_mqqt_data ? false : true;

        if(bcd_show_mqqt_data){
            int result_code = loadMqqtData();
            if(result_code != 0){
                vibrate(1, 1000);
            }
        }

        RTC.read(currentTime);
        showWatchFace(false);

        return;
    }

    if(IS_BTN_RIGHT_DOWN){
        //RTC.read(currentTime);
        //vibTime();
        //return;

        vibrate();
        uint8_t result_code = openDoor();
        if(result_code <= 0){
            vibrate();
        } else {
            vibrate(1, 1000);
        }
    }

    Watchy::handleButtonPress();
}



void BCDWatchyBase::_minutesToHM(int16_t minutes, uint8_t &h, uint8_t &m) {
    uint32_t t = minutes * 60;
    uint8_t s = t % 60;

    t = (t - s)/60;
    m = t % 60;

    t = (t - m)/60;
    h = t;
}


void BCDWatchyBase::vibrate(uint8_t times, uint32_t delay_time){
    pinMode(VIB_MOTOR_PIN, OUTPUT);
    for(uint8_t i=0; i<times; i++){
        delay(delay_time);
        digitalWrite(VIB_MOTOR_PIN, true);
        delay(delay_time);
        digitalWrite(VIB_MOTOR_PIN, false);
    }
}


static void callback(char* topic, byte* payload, unsigned int length){
#if !WATCHY_BCD_HAS_PUBSUBCLIENT
    (void)topic;
    (void)payload;
    (void)length;
    return;
#else
    char message_buf[length+1];
    for (int i = 0; i<length; i++) {
        message_buf[i] = payload[i];
    }
    message_buf[length] = '\0';
    const char *p_payload = message_buf;

    if(strcmp("weather/indoor/temperature", topic) == 0){
        MQTT_RECEIVED_DATA;
        bcd_indoor_temp = atof(p_payload);
    }

    if(strcmp("weather/indoor/zimmer von david/co2", topic) == 0){
        MQTT_RECEIVED_DATA;
        bcd_indoor_co2 = atof(p_payload);
    }

    if(strcmp("weather/indoor/aussen/temperature", topic) == 0){
        MQTT_RECEIVED_DATA;
        bcd_outdoor_temp = atof(p_payload);
    }

    if(strcmp("weather/indoor/wind/windstrength", topic) == 0){
        MQTT_RECEIVED_DATA;
        bcd_outdoor_wind = atoi(p_payload);
    }

    if(strcmp("weather/indoor/wind/guststrength", topic) == 0){
        MQTT_RECEIVED_DATA;
        bcd_outdoor_gusts = atoi(p_payload);
    }

    if(strcmp("weather/indoor/regenmesser/rain", topic) == 0){
        MQTT_RECEIVED_DATA;
        bcd_outdoor_rain = atoi(p_payload);
    }
#endif
}


bool BCDWatchyBase::connectWiFi(){
    int overall_retries = 5;
    while(overall_retries > 0){

        WIFI_CONFIGURED = false;
        WiFi.begin(WIFI_SSID, WIFI_PASS);
        delay(1000);

        int8_t retries = 3;
        while (!WIFI_CONFIGURED && retries > 0) {
            delay(1000);
            WIFI_CONFIGURED = (WiFi.status() == WL_CONNECTED);
            retries--;
        }

        if(WIFI_CONFIGURED){
            break;
        }
        overall_retries--;
    }

    return WIFI_CONFIGURED;
}

void BCDWatchyBase::disconnectWiFi(){
    WIFI_CONFIGURED=false;
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
    btStop();
}


uint8_t BCDWatchyBase::loadMqqtData(){
#if !WATCHY_BCD_HAS_PUBSUBCLIENT
    return 4;
#else
    MQTT_CLEAR;
    vibrate();

    if(!connectWiFi()){
        return 1;
    }

    WiFiClient wifi_client;
    PubSubClient mqtt_client(wifi_client);
    mqtt_client.setServer(MQTT_BROKER, 1883);
    mqtt_client.setCallback(callback);

    mqtt_client.connect("WatchyDavid");
    if(!mqtt_client.connected()){
        disconnectWiFi();
        return 2;
    }

    mqtt_client.subscribe("weather/indoor/temperature");
    mqtt_client.subscribe("weather/indoor/aussen/temperature");
    mqtt_client.subscribe("weather/indoor/wind/windstrength");
    mqtt_client.subscribe("weather/indoor/regenmesser/rain");
    mqtt_client.subscribe("weather/indoor/wind/guststrength");
    mqtt_client.subscribe("weather/indoor/zimmer von david/co2");

    int8_t retries=25;
    while(!MQTT_RECEIVED_ALL_DATA){
        mqtt_client.loop();

        if(retries % 5 == 0){
            vibrate();
        }

        if(retries < 0){
            break;
        }
        retries--;
        delay(100);
    }

    mqtt_client.disconnect();
    disconnectWiFi();

    if(!MQTT_RECEIVED_ALL_DATA){
        return 3;
    }

    return 0;
#endif
}


// https://github.com/espressif/arduino-esp32/issues/3659
uint8_t BCDWatchyBase::openDoor(){
#if !WATCHY_BCD_HAS_PUBSUBCLIENT
    return 4;
#else
    if(!connectWiFi()){
        return 1;
    }

    WiFiClient wifi_client;
    PubSubClient mqtt_client(wifi_client);
    mqtt_client.setServer(MQTT_BROKER, 1883);

    int8_t retries = 20;
    while(!mqtt_client.connected()){
        if(retries < 0){
            break;
        }
        retries--;

        mqtt_client.connect("WatchyDavid");
        delay(250);
    }

    int result = 0;
    if(mqtt_client.connected()){
        mqtt_client.publish(MQTT_TOPIC, MQTT_PAYLOAD);
        mqtt_client.loop();
        mqtt_client.disconnect();
    } else {
        result = 2;
    }

    disconnectWiFi();
    return result;
#endif
}


void BCDWatchyBase::drawHelperGrid(){
    for(int i=0; i<=200; i+=20){
        UiSDK::drawLine(display, i,0,i,200,FOREGROUND_COLOR);
        UiSDK::drawLine(display, 0,i,200,i,FOREGROUND_COLOR);
    }
}


void BCDWatchyBase::drawWatchFace(){
    UiSDK::setRotation(display, bcd_rotation);
    UiSDK::initScreen(display);
    UiSDK::setTextColor(display, FOREGROUND_COLOR);

    if(bcd_sleep_mode){
        UiSDK::drawBitmap(display, 0, 0, sleep_img, 200, 200, FOREGROUND_COLOR);
        return;
    }

    if(bcd_show_mqqt_data){
#if WATCHY_BCD_HAS_PUBSUBCLIENT
        int16_t  x1, y1;
        uint16_t w, h;
        //drawHelperGrid();
        UiSDK::drawBitmap(display, 0, 0, smart_home, 200, 200, FOREGROUND_COLOR);
        UiSDK::setFont(display, &FONT_SMALL);
        UiSDK::getTextBounds(display, String(bcd_indoor_temp), 100, 180, &x1, &y1, &w, &h);
        UiSDK::setCursor(display, 55-w/2, 140);
        UiSDK::print(display, bcd_indoor_temp);
        UiSDK::println(display, "C");

        UiSDK::setCursor(display, 116, 170);
        UiSDK::print(display, bcd_indoor_co2);
        UiSDK::println(display, " ppm");
        UiSDK::drawLine(display, 115, 165, 90, 150, FOREGROUND_COLOR);

        UiSDK::setCursor(display, 130, 120);
        UiSDK::print(display, bcd_outdoor_temp);
        UiSDK::println(display, "C");

        UiSDK::getTextBounds(display, String(bcd_indoor_temp), 100, 180, &x1, &y1, &w, &h);
        UiSDK::setCursor(display, 155-w/2, 40);
        UiSDK::print(display, bcd_outdoor_rain);
        UiSDK::println(display, " mm");

        UiSDK::setCursor(display, 10, 25);
        UiSDK::print(display, bcd_outdoor_wind);
        UiSDK::println(display, " km/h");
        UiSDK::setCursor(display, 10, 45);
        UiSDK::print(display, bcd_outdoor_gusts);
        UiSDK::println(display, " km/h");

        if(!MQTT_RECEIVED_ALL_DATA){
            UiSDK::setCursor(display, 165, 195);
            UiSDK::println(display, "[old]");
        }
#else
        UiSDK::setFont(display, &FONT_SMALL);
        UiSDK::setCursor(display, 10, 30);
        UiSDK::print(display, "MQTT disabled");
#endif
    }
}


uint8_t BCDWatchyBase::getBattery(){
    float voltage = getBatteryVoltage() + BATTERY_OFFSET;

    uint8_t percentage = 2808.3808 * pow(voltage, 4)
                        - 43560.9157 * pow(voltage, 3)
                        + 252848.5888 * pow(voltage, 2)
                        - 650767.4615 * voltage
                        + 626532.5703;
    percentage = min((uint8_t) 100, percentage);
    percentage = max((uint8_t) 0, percentage);
    return percentage;
}


void BCDWatchyBase::drawPixel(int16_t x, int16_t y,uint16_t col){
    if(x > 200 || y > 200 || x < 0 || y < 0){
        return;
    }

    uint16_t real_color;
    switch (col){
        case GREY:
            real_color = (x+y)%2==0 ? GxEPD_WHITE : GxEPD_BLACK;
            break;

        case DARK_GREY:
            real_color = (x+y)%4==0 ? GxEPD_WHITE : GxEPD_BLACK;
            break;

        case LIGHT_GREY:
            real_color = (x+y)%4==0 ? GxEPD_BLACK : GxEPD_WHITE;
            break;

        default:
            real_color = col;
            break;
    }

    UiSDK::drawPixel(display, x, y, real_color);
}

void BCDWatchyBase::drawBitmapCol(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color1){
  int16_t i, j, byteWidth = (w + 7) / 8;
  for(j=0; j<h; j++) {
    for(i=0; i<w; i++ ) {
      if((pgm_read_byte(bitmap + j * byteWidth + i / 8) & (128 >> (i & 7)))==0) {
        drawPixel(x+i, y+j, color1);
      }
    }
  }
}


int BCDWatchyBase::getPixel(int16_t x, int16_t y, const uint8_t *bitmap){
  int16_t imageWidth = pgm_read_byte(bitmap);
  int16_t byteWidth = (imageWidth + 7) / 8;
  return (pgm_read_byte(bitmap + 2 + y * byteWidth + x / 8) & (128 >> (x & 7)));
}


void BCDWatchyBase::drawBitmapRotate(int xx, int yy, const uint8_t *bitmap, unsigned int fAngle, uint16_t color=GxEPD_BLACK){

  int iWidth = pgm_read_byte(bitmap);
  int iHeight = pgm_read_byte(bitmap + 1);
  int hX = iWidth/2;
  int hY = iHeight;
  float angle = fAngle * PI / 180.0;

  int startX = -hX;
  int endX = startX + iWidth;
  int startY = -hY;
  int endY = startY + iHeight;

  for (int x = 0; x < 200; x++) {
    yield();
    for (int y = 0; y < 200; y++) {
      int ux = (x-xx) * cos(-angle) - (y-yy) * sin(-angle);
      int uy = (x-xx) * sin(-angle) + (y-yy) * cos(-angle);

      if(ux >= startX && ux <= endX && uy >= startY && uy <= endY){
        if(!getPixel(ux + hX, uy + hY, bitmap)){
          drawPixel(x, y, color);
        }
      }
    }
  }
}



void BCDWatchyBase::_rtcConfig(){
    RTC.init();
    RTC.clearAlarm();
    RTC.read(currentTime);
}


uint16_t BCDWatchyBase::_readRegister(uint8_t address, uint8_t reg, uint8_t *data, uint16_t len)
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

uint16_t BCDWatchyBase::_writeRegister(uint8_t address, uint8_t reg, uint8_t *data, uint16_t len)
{
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(data, len);
    return (0 !=  Wire.endTransmission());
}
