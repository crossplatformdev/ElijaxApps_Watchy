# How to take a screenshot of the Watchy screen

Open Arduino/libraries/GxEPD2/src/GxEPD2_BW.h in the GxEPD2 library and add the following lines in the `display()` function

    ```c
    Serial.print("#####");
    for(int i=0; i<5000; i++){
      Serial.print("0x");
      if(_buffer[i] <= 0x0F){
        Serial.print("0");
      }
      Serial.print(_buffer[i], HEX);
      Serial.print(", ");
      Serial.print("*****")
    }
    ```

Within your watch face `setup()` function

    ```c
    Serial.begin(115200);
    while (!Serial) {
      ; // wait for serial port to connect. Needed for native USB port only
      }
    ```

This should print out the buffer in the serial terminal

* Copy the output and paste it [here](https://javl.github.io/image2cpp/)
* Set the size to 200×200 px
* Click on `Read as horizontal`
