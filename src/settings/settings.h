#ifndef SETTINGS_H
#define SETTINGS_H

#include "../watchy/Watchy.h"

// Weather Settings
#define CITY_ID "3117735" // Madrid https://openweathermap.org/current#cityid

// Compatibility defines used by some upstream Watchy watchfaces that build the
// OpenWeatherMap query using city+country instead of CITY_ID or LAT/LON.
// This firmware primarily uses CITY_ID (or LAT/LON) via OPENWEATHERMAP_URL, but
// we keep these around so upstream ports compile without needing per-face config.
#ifndef CITY_NAME
#define CITY_NAME "MADRID"
#endif
#ifndef COUNTRY_CODE
#define COUNTRY_CODE "ES"
#endif

// You can also use LAT,LON for your location instead of CITY_ID, but not both
// #define LAT "40.7127" // New York City, Looked up on https://www.latlong.net/
// #define LON "-74.0059"

#define OPENWEATHERMAP_APIKEY "25819493fef88b03ed48312e7436448b" // use your own API key :)

#ifdef CITY_ID
    #define OPENWEATHERMAP_URL "http://api.openweathermap.org/data/2.5/weather?id={cityID}&lang={lang}&units={units}&appid={apiKey}" // open weather api using city ID
#else
    #define OPENWEATHERMAP_URL "http://api.openweathermap.org/data/2.5/weather?lat={lat}&lon={lon}&lang={lang}&units={units}&appid={apiKey}" // open weather api using lat lon
#endif

#define TEMP_UNIT "metric" // metric = Celsius , imperial = Fahrenheit
#define TEMP_LANG "en"
#define WEATHER_UPDATE_INTERVAL 6 // must be greater than 5, measured in minutes

// NTP Settings
#define NTP_SERVER "0.debian.pool.ntp.org"
#define GMT_OFFSET_SEC (3600 * 1) // New York is UTC -5 EST, -4 EDT, will be overwritten by weather data

extern watchySettings settings;

// Initial RTC seed time. Format expected by Watchy32KRTC::config: YYYY:MM:DD:HH:MM:SS
#define DATETIME_NOW "2025:12:14:00:00:00"


// ---- Watchface legacy settings (auto-merged) ----
#ifndef DST_OFFSET_SEC
#define DST_OFFSET_SEC 3600 * -0 // set to -1 for winter time
#endif
#ifndef PING
#define PING 10 //see if network is available
#endif
#ifndef WAKE_UP_INTERVAL_MINUTES
#define WAKE_UP_INTERVAL_MINUTES (0.5/60.0)
#endif
#ifndef LORENZ_POINTS_PER_UPDATE
#define LORENZ_POINTS_PER_UPDATE 5
#endif
#ifndef LORENZ_MAX_POINTS
#define LORENZ_MAX_POINTS 500
#endif
#ifndef LORENZ_ROTATION_SPEED
#define LORENZ_ROTATION_SPEED 0.0523598776
#endif
#ifndef FULL_REFRESH_INTERVAL
#define FULL_REFRESH_INTERVAL 180 // 180 = 90 seconds
#endif
#ifndef TIMEZONE_OFFSET_HOURS
#define TIMEZONE_OFFSET_HOURS 1  // Amsterdam (CET) - change to 2 for CEST (summer time)
#endif
#ifndef LATITUDE
#define LATITUDE 52.3676   // Latitude in degrees (positive = North, negative = South)
#endif
#ifndef LONGITUDE
#define LONGITUDE 4.9041   // Longitude in degrees (positive = East, negative = West)
#endif
#ifndef ipWhoUrl
#define ipWhoUrl "http://ipwho.is/?fields=region,latitude,longitude,timezone.offset"
#endif
#ifndef openMeteoUrl
#define openMeteoUrl "https://api.open-meteo.com/v1/forecast?latitude={lat}&longitude={lon}&daily=temperature_2m_max,temperature_2m_min,weather_code&past_days=7&forecast_days=16&timezone=auto"
#endif
#ifndef LAT
#define LAT "52.007212" //New York City, Looked up on https://www.latlong.net/
#endif
#ifndef LON
#define LON "-0.981142"
#endif
#endif // SETTINGS_H
