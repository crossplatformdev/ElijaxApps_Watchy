# Shadow Clock
![alt text](watchface.gif "This is what you will get")

This is a watch face for the open E-Ink display smartwatch [Watchy](https://watchy.sqfmi.com/).

## Features

- Display time.
- Display battery state as dark ring surrounding clock and light sources angle

## Upload instructions

### I did not use Arduino IDE

I implemented Watchy Sundial using the [platformIO](https://platformio.org/) extension for VS Code, as I can't bear coding in the Arduino IDE anymore. I'm reasonably certain you can just clone this repo and open the project in any platformIO enabled IDE. All necessary dependencies should be installed automatically.  
I even added a `min_spiffs.csv` file and referenced it in the `platformio.ini`, which I think assures the compiler there's enough space on the ESP32 for all those bitmaps and stuff.  
What I want to say is: it's entirely possible platformIO will just compile and upload the code to your Watchy (but see right below first!).

## Compiliation for different Watchy versions

Change `build_flags` in `platformio.ini` to match your Watchy version.

## pic2array

Converts image into 8bit monochrome byte array.

It requires 2 parameters:
- source image
- output path

## picNormal2array

Converts normal map into 8bit byte array.

It requires 2 parameters:
- source image
- output path

## Thanks and contributions

### mehtmehtsen - creator of BotWatchy face

I based this project on his [BotWatchy](https://github.com/mehtmehtsen/BotWatchy).

###  jyvet - creator of pic2oled
https://github.com/jyvet/pic2oled

I've modified their python script to generate 8bit monochromatic byte array
