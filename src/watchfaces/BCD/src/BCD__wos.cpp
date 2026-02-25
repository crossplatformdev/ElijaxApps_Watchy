#include "Kernel.h"
Kernel kernel;




#ifdef WATCHY_STANDALONE_WATCHFACE
void setup(){
  kernel.state_machine();
}


void loop(){

}

#endif
