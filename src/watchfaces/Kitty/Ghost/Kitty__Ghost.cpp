#include "Ghost.h"

#ifdef WATCHY_STANDALONE_WATCHFACE
Ghost watchy;

void setup() {
  watchy.init();
}

void loop() {}
#endif