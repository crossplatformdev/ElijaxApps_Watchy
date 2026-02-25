#ifndef SETTINGS_H
#define SETTINGS_H

#include "Watchy.h"

//Weather Settings
#define CITY_ID "6545083" //Madrid https://openweathermap.org/current#cityid

//You can also use LAT,LON for your location instead of CITY_ID, but not both
#define LAT "40.4168" //Madrid, Looked up on https://www.latlong.net/
#define LON "-3.7038" //Madrid, Looked up on https://www.latlong.net/

#ifdef CITY_ID
    #define OPENWEATHERMAP_URL "http://api.openweathermap.org/data/2.5/weather?id={cityID}&lang={lang}&units={units}&appid={apiKey}" //open weather api using city ID
#else
    #define OPENWEATHERMAP_URL "http://api.openweathermap.org/data/2.5/weather?lat={lat}&lon={lon}&lang={lang}&units={units}&appid={apiKey}" //open weather api using lat lon
#endif

#define OPENWEATHERMAP_APIKEY "25819493fef88b03ed48312e7436448b" //use your own API key :)
#define TEMP_UNIT "metric" //metric = Celsius , imperial = Fahrenheit
#define TEMP_LANG "en"
#define WEATHER_UPDATE_INTERVAL 30 //must be greater than 5, measured in minutes
//NTP Settings
#define NTP_SERVER "pool.ntp.org"
#define GMT_OFFSET_SEC 3600 * 2

extern watchySettings settings;

#endif