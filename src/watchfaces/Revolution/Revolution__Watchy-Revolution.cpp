#include "const.h"
#include "revolution.h"

static Revolution watchy(
    // Display the year using roman numerals
    true,
    // Size of the analog clock hands
    2.0,
    // Name of the days in French or English
    FrenchRepublicanCalendar::Language::French);



#ifdef WATCHY_STANDALONE_WATCHFACE
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

#endif
