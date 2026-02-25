#include "settings.h"

watchySettings settings{
#ifdef CITY_ID
    .cityID = CITY_ID,
#else
    .cityID = "",
    .lat = LAT,
    .lon = LON,
#endif
    .weatherAPIKey = OPENWEATHERMAP_APIKEY,
    .weatherURL = OPENWEATHERMAP_URL,
    .weatherUnit = TEMP_UNIT,
    .weatherLang = TEMP_LANG,
    .weatherUpdateInterval = WEATHER_UPDATE_INTERVAL,
    .ntpServer = NTP_SERVER,
    .gmtOffset = GMT_OFFSET_SEC,
    .vibrateOClock = true,
};
