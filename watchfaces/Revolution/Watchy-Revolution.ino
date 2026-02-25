#include "const.h"
#include "revolution.h"

Revolution watchy(
    // Dark mode (if enabled, white on black, otherwise black on white)
    true,
    // Display the year using roman numerals
    true,
    // Size of the analog clock hands
    2.0,
    // Name of the days in French or English
    FrenchRepublicanCalendar::Language::French);

void setup()
{
#ifdef DEBUG
    Serial.begin(115200);
#endif
    watchy.init();
}

void loop()
{
}
