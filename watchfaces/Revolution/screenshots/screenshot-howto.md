# How to take a screenshot of the Watchy screen

* Open [GxEPD2_BW.h](https://github.com/ZinggJM/GxEPD2/blob/74079a8ca692e5089463b3a02526a370391b8cc8/src/GxEPD2_BW.h#L170) in the GxEPD2 library and add the following lines in the `display()` function

    ```c
    for(int i=0; i<5000; i++){
      Serial.print("0x");
      if(_buffer[i] <= 0x0F){
        Serial.print("0");
      }
      Serial.print(_buffer[i], HEX);
      Serial.print(", ");
    }
    ```

    This should print out the buffer in the serial terminal

* Copy the output and paste it [here](https://javl.github.io/image2cpp/)
* Set the size to 200×200 px
* Click on `Read as horizontal`
