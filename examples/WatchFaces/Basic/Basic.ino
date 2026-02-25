#include <WatchySdk.h>
#include "settings.h"

void setup(){
  WatchySdk::settings = settings;
  WatchySdk::init();
}

void loop(){}