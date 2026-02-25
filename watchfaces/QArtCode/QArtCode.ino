#include "Watchy_QArtCode.h"

WatchyQArtCode watchy;

void setup(){
  Serial.begin(9600);
  watchy.init();
}

void loop(){}
