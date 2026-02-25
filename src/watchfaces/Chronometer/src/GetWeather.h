#pragma once

#include "../../../watchy/Watchy.h"

namespace Watchy_GetWeather {

constexpr const char* OPENWEATHERMAP_APIKEY =
    "f058fe1cad2afe8e2ddc5d063a64cecb";  // use your own API key :)
constexpr const char* OPENWEATHERMAP_URL =
    "http://api.openweathermap.org/data/2.5/weather";
constexpr const char* TEMP_UNIT = "metric";  // use "imperial" for Fahrenheit"
const int WEATHER_UPDATE_INTERVAL = 30 * 60 * 1000;  // 30 minutes in millis

typedef struct weatherData {
  int8_t temperature;
  int16_t weatherConditionCode;
  char weatherCity[25];
} weatherData;

extern weatherData getWeather();
extern time_t lastGetWeatherTS; // timestamp of last successful getWeather call

}  // namespace Watchy_GetWeather
