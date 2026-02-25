# Revolution Watchface for [Watchy](https://watchy.sqfmi.com/)

![Watchface GIF](screenshots/watchfaces.gif)

[![Build](https://github.com/jeandeaual/Watchy-Revolution/actions/workflows/build.yml/badge.svg)](https://github.com/jeandeaual/Watchy-Revolution/actions/workflows/build.yml)

This is a watch face for the open-source e-paper smartwatch [Watchy](https://watchy.sqfmi.com/).

It shows the current date in the [French Republican calendar](https://en.wikipedia.org/wiki/French_Republican_calendar) (also called the French Revolutionary calendar)
and the current [decimal time](https://en.wikipedia.org/wiki/Decimal_time).

Leap years are calculated using the Romme method: leap days are inserted at the end of each year divisible by four, except century years not divisible by 400, as part of that year.

## Usage

By pressing up or down (top-right and bottom-right buttons), it is possible to cycle through 4 modes:

1. Digital time with the current date

    ![Digital date watchface screenshot](screenshots/digital-date.png)

1. Digital time only

    ![Digital watchface screenshot](screenshots/digital.png)

1. Analog time with the current date

    ![Analog date watchface screenshot](screenshots/analog-date.png)

1. Analog time only

    ![Analog watchface screenshot](screenshots/analog.png)

## Customization

See [Watchy-Revolution.ino](https://github.com/jeandeaual/Watchy-Revolution/blob/master/Watchy-Revolution.ino#L4) for some customization options:

* Dark or light mode
* Size of the analog clock hands
* Name of the days in French or English

    By default the [name of each day in the year](https://en.wikipedia.org/wiki/French_Republican_calendar#Rural_calendar) is written in French.

## Build

### Using [arduino-cli](https://arduino.github.io/arduino-cli/latest/installation/)

* Install the ESP32 platform:

    ```sh
    arduino-cli core update-index --additional-urls https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
    arduino-cli core install esp32:esp32 --additional-urls https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
    ```

* Install the [Watchy library](https://github.com/sqfmi/Watchy):

    ```sh
    arduino-cli lib install Watchy@1.2.6
    ```

* Compile the sketch:

    ```sh
    arduino-cli compile --fqbn esp32:esp32:esp32 --build-property build.partitions=min_spiffs --build-property upload.maximum_size=1966080 Watchy-Revolution.ino
    ```

* Upload the firmware to the watch:

   ```sh
   arduino-cli upload -p /dev/ttyACM0 --fqbn esp32:esp32:esp32 Watchy-Revolution
   ```

   Replace `/dev/ttyACM0` with the port listed by `arduino-cli board list`.

### Using the [Arduino IDE](https://www.arduino.cc/en/software)

* See <https://watchy.sqfmi.com/docs/getting-started#arduino-setup> and <https://watchy.sqfmi.com/docs/getting-started#upload>.

* Open `Watchy-Revolution.ino`.

* Click on `Upload`.

## Acknowledgements

* Thanks to the [Calculateur](https://git.sr.ht/~jochen/Calculateur) watchface for the idea and the DS3231 RTC Alarm1 usage.
* This project uses the [Libertinus Serif](https://github.com/alerque/libertinus) font.
