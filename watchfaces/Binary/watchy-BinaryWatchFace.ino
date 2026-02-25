#include "../../src/watchy/Watchy.h" //include the Watchy library
#include <Fonts/FreeMonoBold18pt7b.h> //include any fonts you want to use
#include <circuit_board.h>

class MyFirstWatchFace : public Watchy{ //inherit and extend Watchy class
    public:
        void drawWatchFace(){ //override this method to customize how the watch face looks
          display.drawBitmap(0, 0, circuit_board, 200, 200, GxEPD_WHITE);

          display.setFont(&FreeMonoBold18pt7b);
          display.setCursor(35, 95);
          switch (currentTime.Hour) {
            case 0: case 1:
              display.print("00000");
              break;
            case 2: case 3:
              display.print("0000");
              break;
            case 4: case 5: case 6: case 7:
              display.print("000");
              break;
            case 8: case 9: case 10: case 11: case 12: case 13: case 14: case 15:
              display.print("00");
              break;
            case 16: case 17: case 18: case 19: case 20: case 21: case 22: case 23: case 24:
              display.print("0");
              break;
            default:
              display.print("Fehler bei Hour");
              break; // Wird nicht benötigt, wenn Statement(s) vorhanden sind
          }
          display.println(currentTime.Hour, BIN);
          
          display.setCursor(35,125);
          if(currentTime.Minute <= 31 && currentTime.Minute > 15){
            display.print("0");
          }
          else if(currentTime.Minute <= 15 && currentTime.Minute > 7){
            display.print("00");
          }
          else if(currentTime.Minute <= 7 && currentTime.Minute > 3){
            display.print("000");
          }
          else if(currentTime.Minute <= 3 && currentTime.Minute > 1){
            display.print("0000");
          }
          else if(currentTime.Minute <= 1){
            display.print("00000");
          }
          display.print(currentTime.Minute, BIN);  
        }
};

MyFirstWatchFace m; //instantiate your watchface

void setup() {
  m.init(); //call init in setup
}

void loop() {
  // this should never run, Watchy deep sleeps after init();
}
