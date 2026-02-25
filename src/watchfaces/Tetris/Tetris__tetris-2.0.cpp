#include "Watchy_Tetris.h"
#include "../../settings/settings.h"

static WatchyTetris watchy(settings);



#ifdef WATCHY_STANDALONE_WATCHFACE
void setup()
{
    watchy.init();
}

void loop() {}
#endif
