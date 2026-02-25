# CALCULATEUR: A decimal watch face for SQFMI's Watchy

![Watchy](https://jochen.link/experiments/img/calculateur/calculateur-watch-face.jpg)

In contrast to common clocks, **Calculateur** tells the usable time ahead, mapping each day to 1000 decimal minutes, counting down.

Visit [**https://jochen.link/experiments/calculateur.html**](https://jochen.link/experiments/calculateur.html) for more information and to check out the web version or download the Chrome extension.



## CHECK OUT WATCHY: Fully Open Source E-Paper Watch:
Please visit [**https://watchy.sqfmi.com**](https://watchy.sqfmi.com) for documentation, hardware design files, and more!

**Calculateur** is to 99.9% the fantastic work of SQFMI with some minor, messy modifications to the original Watchy library by me. For **Calculateur** to function it was necessary to use DS3232RTC Alarm1, waking it up only every 86.4 seconds.

Besides enabling the actual watch face, it'll also keep your Watchy running for longer :)



## SETUP
1. In the Arduino IDE Boards Manager, install support for the ESP32. You can find instructions here: https://github.com/espressif/arduino-esp32/blob/master/docs/arduino-ide/boards_manager.md
2. Install the original Watchy library (search for **Watchy** in the library manager), and any other depdencies when prompted!
3. Compile & Upload with these board settings:
    * Board: "ESP32 DEv Module"
    * Partition Scheme: "Minimal SPIFFS"
    * All other Settings: leave to default

If you don't install the original Watchy library you'll need following dependencies:
- Adafruit_BusIO
- Adafruit_GFX_Library
- Arduino_JSON
- DS3232RTC
- GxEPD2
- Time
- Ultrasonic
- WiFiManager

**PS: sorry for the mess :)**