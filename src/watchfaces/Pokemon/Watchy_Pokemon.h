#ifndef WATCHY_POKEMON_H
#define WATCHY_POKEMON_H

//#define FR
//#define SIM

#include "../../watchy/Watchy.h"
#include "wta.h"

#include "FreeMonoBold10pt7b.h"
#include "FreeMonoBold7pt7b.h"

#include "pokemon_yellow.h"
//#include "pokemon_red_blue.h"
//#include "pokemon_red_green.h"

#include "ui_other.h"

#ifdef FR
#include "ui_fr.h"
#else
#include "ui_en.h"
#endif

#ifdef SIM
class WatchyPokemon : public Watchy
#else
class WatchyPokemon : public WatchySynced
#endif
{
#ifndef SIM
    using WatchySynced::WatchySynced;
#endif
public:
    void drawWatchFace();
    double randomDay(uint32_t d);
    double randomHour(uint32_t d);
    double randomMinute(uint32_t d);
};

#endif
