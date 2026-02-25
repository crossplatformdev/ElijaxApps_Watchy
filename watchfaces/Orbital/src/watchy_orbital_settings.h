#ifndef WATCHY_ORBITAL_SETTINGS_H
#define WATCHY_ORBITAL_SETTINGS_H

#include "watchy_orbital.h"

// Latitude and longitude of your position. Used for sunrise/sunset and moon phase calculation
float lat = 48.8534;
float lon = 2.3488;

// Translations of day of the week, starting at sunday (and ending at sunday too)
const char* daysOfWeek[] = {"dimanche", "lundi", "mardi", "mercredi", "jeudi", "vendredi", "samedi", "dimanche"};
// Very short month names, january to december. For the "details" mode, so should be short to be shown on the circle
const String shortMonths[] = {"ja", "fe", "ma", "av", "ma", "ju", "ji", "ao", "se", "oc", "no", "de"};

/* Values of voltage considered as max (100%) and min (0%), to display the central circle.
 * Value is in milli-volt.
 * The default value is usually 3800 to 4100, but I found that my battery was more between 3500 and 4200. YMMV, hence the setting.
 */
int batteryVMax = 4200;
int batteryVMin = 3500;

/* Startup mode. Leave as is if you don't know what it is.
 * 8 modes
 * 0 : white on black, no details, 12h
 * 1 : white on black, no details, 24h
 * 2 : white on black, details, 12h
 * 3 : white on black, details, 24h
 * 4 : black on white, no details, 12h
 * 5 : black on white, no details, 24h
 * 6 : black on white, details, 12h
 * 7 : black on white, details, 24h
 */
RTC_DATA_ATTR int mode = 0;

#endif
