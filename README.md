# Watchy Application Suite

A complete application-focused firmware for the Watchy open-source e-paper
watch. It provides **142 applications across 10 categories**, a consistent
Watchy OS interface, deterministic public screenshots, and a lightweight SDK
for adding more applications.

The screenshots below come directly from the real `200x200`, 1-bit firmware
framebuffer. They are not UI mockups. Public-safe fixtures provide the fixed
time, sensor readings, health profile, locations, and network results shown in
the gallery; normal firmware uses live hardware and configured data.

## Watchy OS

Watchy OS presents every feature through a two-level category menu with
consistent typography, selection, input, themes, and display lifecycle. The
root view provides access to all ten application groups.

<p align="center"><img src="docs/gallery/png/os/menu/categories/light.png" alt="Watchy OS category menu" width="200"></p>

| Category | Applications |
| --- | ---: |
| System | 5 |
| Utilities | 23 |
| Networking | 9 |
| Astronomy | 4 |
| Healthcare | 35 |
| Games | 15 |
| Clocks | 12 |
| Time Tools | 6 |
| Sensors | 17 |
| Bluetooth | 16 |
| **Total** | **142** |

[System](#system) · [Utilities](#utilities) · [Networking](#networking) ·
[Astronomy](#astronomy) · [Healthcare](#healthcare) · [Games](#games) ·
[Clocks](#clocks) · [Time Tools](#time-tools) · [Sensors](#sensors) ·
[Bluetooth](#bluetooth)

## System

Core Watchy OS configuration, identity, connectivity, watch-face selection,
and appearance controls.

<p align="center"><img src="docs/gallery/png/os/menu/system/light.png" alt="System menu" width="200"></p>

| Application | What it does | Screenshot |
| --- | --- | --- |
| **About Watchy** | Displays firmware, hardware, and project information. | <img src="docs/gallery/png/system/about-watchy/default/light.png" alt="About Watchy" width="200"> |
| **Set Time** | Provides an on-watch editor for the local date and time. | <img src="docs/gallery/png/system/set-time/default/light.png" alt="Set Time" width="200"> |
| **Setup WiFi** | Configures Wi-Fi and reports the resulting connection state. | <img src="docs/gallery/png/system/setup-wifi/connected/light.png" alt="Setup WiFi" width="200"> |
| **Watch Faces** | Selects the active watch face from the installed collection. | <img src="docs/gallery/png/system/watch-faces/selector/light.png" alt="Watch Faces" width="200"> |
| **Theme Colours** | Switches the shared Watchy OS interface between light and dark themes. | <img src="docs/gallery/png/system/theme-colours/light-selected/light.png" alt="Theme Colours" width="200"> |

## Utilities

Everyday randomizers, diagnostics, hardware tools, generators, and unit
converters.

<p align="center"><img src="docs/gallery/png/os/menu/utilities/light.png" alt="Utilities menu" width="200"></p>

| Application | What it does | Screenshot |
| --- | --- | --- |
| **Vibrate Motor** | Runs a short haptic motor test from the Utilities menu. | <img src="docs/gallery/png/utilities/vibrate-motor/default/light.png" alt="Vibrate Motor" width="200"> |
| **Accelerometer** | Shows the current three-axis accelerometer reading and orientation. | <img src="docs/gallery/png/utilities/accelerometer/face-up/light.png" alt="Accelerometer" width="200"> |
| **Sync NTP** | Synchronizes the RTC with an internet time source and reports success or failure. | <img src="docs/gallery/png/utilities/sync-ntp/success/light.png" alt="Sync NTP" width="200"> |
| **Coin Flip** | Produces a quick heads-or-tails decision. | <img src="docs/gallery/png/utilities/coin-flip/heads/light.png" alt="Coin Flip" width="200"> |
| **D6 Dice** | Rolls a standard six-sided die. | <img src="docs/gallery/png/utilities/d6-dice/four/light.png" alt="D6 Dice" width="200"> |
| **D20 Dice** | Rolls a twenty-sided tabletop die. | <img src="docs/gallery/png/utilities/d20-dice/seventeen/light.png" alt="D20 Dice" width="200"> |
| **Random Number** | Generates a reusable random numeric value. | <img src="docs/gallery/png/utilities/random-number/default/light.png" alt="Random Number" width="200"> |
| **Decision Maker** | Answers a simple yes-or-no choice. | <img src="docs/gallery/png/utilities/decision-maker/yes/light.png" alt="Decision Maker" width="200"> |
| **Password Generator** | Creates a compact mixed-character password on the watch. | <img src="docs/gallery/png/utilities/password-generator/default/light.png" alt="Password Generator" width="200"> |
| **UUID Generator** | Creates and displays a standards-shaped UUID. | <img src="docs/gallery/png/utilities/uuid-generator/default/light.png" alt="UUID Generator" width="200"> |
| **I2C Scanner** | Probes the I2C bus and lists responding device addresses. | <img src="docs/gallery/png/utilities/i2c-scanner/default/light.png" alt="I2C Scanner" width="200"> |
| **Chip Info** | Summarizes the ESP32-S3, CPU, flash, and SDK configuration. | <img src="docs/gallery/png/utilities/chip-info/default/light.png" alt="Chip Info" width="200"> |
| **Heap Monitor** | Reports free heap, minimum free heap, and largest available block. | <img src="docs/gallery/png/utilities/heap-monitor/default/light.png" alt="Heap Monitor" width="200"> |
| **Wake Reason** | Explains which wake source resumed the watch. | <img src="docs/gallery/png/utilities/wake-reason/timer/light.png" alt="Wake Reason" width="200"> |
| **Reset Reason** | Reports the ESP32 reset cause and raw reason code. | <img src="docs/gallery/png/utilities/reset-reason/power-on/light.png" alt="Reset Reason" width="200"> |
| **Button Tester** | Counts input events so every physical button can be checked. | <img src="docs/gallery/png/utilities/button-tester/default/light.png" alt="Button Tester" width="200"> |
| **Vibration Lab** | Selects and previews repeatable haptic patterns. | <img src="docs/gallery/png/utilities/vibration-lab/heartbeat/light.png" alt="Vibration Lab" width="200"> |
| **Screen Ruler** | Draws a calibrated on-screen ruler for quick measurements. | <img src="docs/gallery/png/utilities/screen-ruler/default/light.png" alt="Screen Ruler" width="200"> |
| **Temperature Converter** | Converts between Celsius and Fahrenheit. | <img src="docs/gallery/png/utilities/temperature-converter/default/light.png" alt="Temperature Converter" width="200"> |
| **Length Converter** | Converts common metric and imperial lengths. | <img src="docs/gallery/png/utilities/length-converter/default/light.png" alt="Length Converter" width="200"> |
| **Weight Converter** | Converts common metric and imperial weights. | <img src="docs/gallery/png/utilities/weight-converter/default/light.png" alt="Weight Converter" width="200"> |
| **Base Converter** | Displays a number in decimal, hexadecimal, octal, and binary. | <img src="docs/gallery/png/utilities/base-converter/default/light.png" alt="Base Converter" width="200"> |
| **Pace Converter** | Converts running pace between common distance units. | <img src="docs/gallery/png/utilities/pace-converter/default/light.png" alt="Pace Converter" width="200"> |

## Networking

Compact network inspection and text-retrieval tools. Scanning tools must only
be used on networks and hosts you are authorized to test.

<p align="center"><img src="docs/gallery/png/os/menu/networking/light.png" alt="Networking menu" width="200"></p>

| Application | What it does | Screenshot |
| --- | --- | --- |
| **Browser** | Fetches and presents a compact text-oriented web page. | <img src="docs/gallery/png/networking/browser/result/light.png" alt="Browser" width="200"> |
| **RSS Feed** | Retrieves a feed and displays recent item titles and summaries. | <img src="docs/gallery/png/networking/rss-feed/result/light.png" alt="RSS Feed" width="200"> |
| **Ping** | Measures reachability, replies, and round-trip latency. | <img src="docs/gallery/png/networking/ping/result/light.png" alt="Ping" width="200"> |
| **Traceroute** | Lists the network hops observed on the path to a host. | <img src="docs/gallery/png/networking/traceroute/result/light.png" alt="Traceroute" width="200"> |
| **Port Scanner** | Checks a focused set of common TCP ports on an authorized host. | <img src="docs/gallery/png/networking/port-scanner/result/light.png" alt="Port Scanner" width="200"> |
| **DNS Query** | Resolves a host name and displays the DNS result and status. | <img src="docs/gallery/png/networking/dns-query/result/light.png" alt="DNS Query" width="200"> |
| **Reverse DNS** | Resolves an IP address back to its PTR host name. | <img src="docs/gallery/png/networking/reverse-dns/result/light.png" alt="Reverse DNS" width="200"> |
| **DuckDuckGo** | Runs a lightweight web search and lists compact results. | <img src="docs/gallery/png/networking/duckduckgo/result/light.png" alt="DuckDuckGo" width="200"> |
| **WiFi Survey** | Lists nearby access points with channel, RSSI, security, and BSSID. | <img src="docs/gallery/png/networking/wifi-survey/result/light.png" alt="WiFi Survey" width="200"> |

## Astronomy

Daily solar, lunar, and tidal information using configured locations and
stations.

<p align="center"><img src="docs/gallery/png/os/menu/astronomy/light.png" alt="Astronomy menu" width="200"></p>

| Application | What it does | Screenshot |
| --- | --- | --- |
| **Sun Rise** | Calculates sunrise, sunset, and daylight information for a location. | <img src="docs/gallery/png/astronomy/sun-rise/madrid/light.png" alt="Sun Rise" width="200"> |
| **Moon Rise** | Calculates moonrise and moonset times for a location. | <img src="docs/gallery/png/astronomy/moon-rise/madrid/light.png" alt="Moon Rise" width="200"> |
| **Moon Phase** | Shows the current lunar phase, age, and illumination. | <img src="docs/gallery/png/astronomy/moon-phase/default/light.png" alt="Moon Phase" width="200"> |
| **Tides** | Displays four daily tide events, coefficients, range, and station data. | <img src="docs/gallery/png/astronomy/tides/brest/light.png" alt="Tides" width="200"> |

## Healthcare

Medical identity, safety monitoring, reminders, emergency references, and
guided wellbeing tools. These features are informational aids and are not a
substitute for professional medical care or emergency services.

<p align="center"><img src="docs/gallery/png/os/menu/healthcare/light.png" alt="Healthcare menu" width="200"></p>

| Application | What it does | Screenshot |
| --- | --- | --- |
| **Heart Rate** | Estimates pulse using experimental accelerometer-based ballistocardiography. | <img src="docs/gallery/png/healthcare/heart-rate/result/light.png" alt="Heart Rate" width="200"> |
| **UN Dog Plate** | Formats emergency identity and medical details in a compact plate-style view. | <img src="docs/gallery/png/healthcare/un-dog-plate/demo/light.png" alt="UN Dog Plate" width="200"> |
| **Medical ID** | Presents a concise emergency summary from the configured medical profile. | <img src="docs/gallery/png/healthcare/medical-id/demo/light.png" alt="Medical ID" width="200"> |
| **ICE Contact** | Displays the configured in-case-of-emergency contact. | <img src="docs/gallery/png/healthcare/ice-contact/demo/light.png" alt="ICE Contact" width="200"> |
| **Blood Type** | Shows the configured blood group prominently for emergency reference. | <img src="docs/gallery/png/healthcare/blood-type/demo/light.png" alt="Blood Type" width="200"> |
| **Allergies** | Lists important configured allergies. | <img src="docs/gallery/png/healthcare/allergies/demo/light.png" alt="Allergies" width="200"> |
| **Medications** | Lists current configured medications. | <img src="docs/gallery/png/healthcare/medications/demo/light.png" alt="Medications" width="200"> |
| **Conditions** | Lists relevant configured medical conditions. | <img src="docs/gallery/png/healthcare/conditions/demo/light.png" alt="Conditions" width="200"> |
| **Edit Medical ID** | Provides an on-watch editor for the emergency medical profile. | <img src="docs/gallery/png/healthcare/edit-medical-id/demo/light.png" alt="Edit Medical ID" width="200"> |
| **Fall Detector** | Monitors motion for a possible fall and supports an emergency response flow. | <img src="docs/gallery/png/healthcare/fall-detector/monitoring/light.png" alt="Fall Detector" width="200"> |
| **Body Position** | Reports body or watch orientation for quick safety assessment. | <img src="docs/gallery/png/healthcare/body-position/face-up/light.png" alt="Body Position" width="200"> |
| **Saved Location** | Displays the configured emergency location details. | <img src="docs/gallery/png/healthcare/saved-location/demo/light.png" alt="Saved Location" width="200"> |
| **SOS Screen** | Shows a high-contrast emergency message and essential identity details. | <img src="docs/gallery/png/healthcare/sos-screen/demo/light.png" alt="SOS Screen" width="200"> |
| **SOS BLE Beacon** | Broadcasts a configured emergency payload over Bluetooth Low Energy. | <img src="docs/gallery/png/healthcare/sos-ble/broadcasting/light.png" alt="SOS BLE Beacon" width="200"> |
| **Check-In Timer** | Starts a safety timer that expects the wearer to acknowledge before expiry. | <img src="docs/gallery/png/healthcare/check-in-timer/armed/light.png" alt="Check-In Timer" width="200"> |
| **Medication Alert** | Enables and displays a recurring medication reminder. | <img src="docs/gallery/png/healthcare/medication-alert/enabled/light.png" alt="Medication Alert" width="200"> |
| **Hydration Alert** | Enables and displays a recurring hydration reminder. | <img src="docs/gallery/png/healthcare/hydration-alert/enabled/light.png" alt="Hydration Alert" width="200"> |
| **Breathing Coach** | Guides paced inhale and exhale intervals. | <img src="docs/gallery/png/healthcare/breathing-coach/inhale/light.png" alt="Breathing Coach" width="200"> |
| **CPR Metronome** | Provides a steady compression cadence reference for CPR. | <img src="docs/gallery/png/healthcare/cpr-metronome/running/light.png" alt="CPR Metronome" width="200"> |
| **Recovery Position** | Presents concise recovery-position guidance. | <img src="docs/gallery/png/healthcare/recovery-position/default/light.png" alt="Recovery Position" width="200"> |
| **Stroke FAST** | Presents the Face, Arms, Speech, Time stroke-recognition checklist. | <img src="docs/gallery/png/healthcare/stroke-fast/default/light.png" alt="Stroke FAST" width="200"> |
| **Choking Response** | Shows concise first-aid steps for a choking emergency. | <img src="docs/gallery/png/healthcare/choking-response/default/light.png" alt="Choking Response" width="200"> |
| **Seizure Aid** | Shows immediate safety guidance for assisting during a seizure. | <img src="docs/gallery/png/healthcare/seizure-aid/default/light.png" alt="Seizure Aid" width="200"> |
| **Severe Bleeding** | Presents first-aid actions for controlling severe bleeding. | <img src="docs/gallery/png/healthcare/severe-bleeding/default/light.png" alt="Severe Bleeding" width="200"> |
| **Burn First Aid** | Presents immediate first-aid guidance for burns. | <img src="docs/gallery/png/healthcare/burn-first-aid/default/light.png" alt="Burn First Aid" width="200"> |
| **Heat Emergency** | Lists recognition and response steps for heat illness. | <img src="docs/gallery/png/healthcare/heat-emergency/default/light.png" alt="Heat Emergency" width="200"> |
| **Hypothermia** | Lists recognition and response steps for dangerous cold exposure. | <img src="docs/gallery/png/healthcare/hypothermia/default/light.png" alt="Hypothermia" width="200"> |
| **Poisoning** | Presents immediate poisoning response guidance and cautions. | <img src="docs/gallery/png/healthcare/poisoning/default/light.png" alt="Poisoning" width="200"> |
| **Anaphylaxis** | Presents urgent response guidance for severe allergic reactions. | <img src="docs/gallery/png/healthcare/anaphylaxis/default/light.png" alt="Anaphylaxis" width="200"> |
| **Opioid Overdose** | Presents recognition and naloxone-oriented emergency guidance. | <img src="docs/gallery/png/healthcare/opioid-overdose/default/light.png" alt="Opioid Overdose" width="200"> |
| **Asthma Attack** | Presents immediate response guidance for an asthma attack. | <img src="docs/gallery/png/healthcare/asthma-attack/default/light.png" alt="Asthma Attack" width="200"> |
| **Emergency Numbers** | Keeps important emergency telephone numbers available offline. | <img src="docs/gallery/png/healthcare/emergency-numbers/default/light.png" alt="Emergency Numbers" width="200"> |
| **Pain Log** | Records a quick pain-intensity entry for later reference. | <img src="docs/gallery/png/healthcare/pain-log/default/light.png" alt="Pain Log" width="200"> |
| **Symptom Note** | Stores a compact symptom note and timestamp. | <img src="docs/gallery/png/healthcare/symptom-note/demo/light.png" alt="Symptom Note" width="200"> |
| **5-4-3-2-1 Calm** | Guides the five-senses grounding exercise one step at a time. | <img src="docs/gallery/png/healthcare/grounding/step-five/light.png" alt="5-4-3-2-1 Calm" width="200"> |

## Games

Fifteen compact games and reaction challenges designed for four-button play on
the e-paper display.

<p align="center"><img src="docs/gallery/png/os/menu/games/light.png" alt="Games menu" width="200"></p>

| Application | What it does | Screenshot |
| --- | --- | --- |
| **Morse Letter** | Challenges the player to identify a letter from Morse code. | <img src="docs/gallery/png/games/morse-letter/answered/light.png" alt="Morse Letter" width="200"> |
| **Morse Code** | Challenges the player to choose the Morse sequence for a letter. | <img src="docs/gallery/png/games/morse-code/answered/light.png" alt="Morse Code" width="200"> |
| **Pong** | Plays a compact paddle-and-ball game. | <img src="docs/gallery/png/games/pong/in-play/light.png" alt="Pong" width="200"> |
| **Snake** | Grows a snake while avoiding walls and its own trail. | <img src="docs/gallery/png/games/snake/in-play/light.png" alt="Snake" width="200"> |
| **Othello** | Implements the classic disk-flipping strategy game. | <img src="docs/gallery/png/games/othello/in-play/light.png" alt="Othello" width="200"> |
| **Rock Paper Scissors** | Plays a scored round against the watch. | <img src="docs/gallery/png/games/rock-paper-scissors/win/light.png" alt="Rock Paper Scissors" width="200"> |
| **Reaction Test** | Measures response time after a randomized start signal. | <img src="docs/gallery/png/games/reaction-test/result/light.png" alt="Reaction Test" width="200"> |
| **Higher Lower** | Guesses whether the next value will be higher or lower. | <img src="docs/gallery/png/games/higher-lower/correct/light.png" alt="Higher Lower" width="200"> |
| **Number Guess** | Narrows down a hidden number using higher and lower hints. | <img src="docs/gallery/png/games/number-guess/too-low/light.png" alt="Number Guess" width="200"> |
| **Nim** | Removes objects strategically while playing against the watch. | <img src="docs/gallery/png/games/nim/in-play/light.png" alt="Nim" width="200"> |
| **Tic Tac Toe** | Plays a compact three-by-three noughts-and-crosses game. | <img src="docs/gallery/png/games/tic-tac-toe/in-play/light.png" alt="Tic Tac Toe" width="200"> |
| **Lights Out** | Toggles a five-by-five grid with the goal of clearing every light. | <img src="docs/gallery/png/games/lights-out/in-play/light.png" alt="Lights Out" width="200"> |
| **Blackjack** | Plays a simplified hand of blackjack against the dealer. | <img src="docs/gallery/png/games/blackjack/in-play/light.png" alt="Blackjack" width="200"> |
| **Quick Math** | Presents short arithmetic questions and tracks correct answers. | <img src="docs/gallery/png/games/quick-math/correct/light.png" alt="Quick Math" width="200"> |
| **Balance** | Uses the accelerometer to keep an indicator centered. | <img src="docs/gallery/png/games/balance/in-play/light.png" alt="Balance" width="200"> |

## Clocks

Alternative representations of civil, global, astronomical, and progress-based
time.

<p align="center"><img src="docs/gallery/png/os/menu/clocks/light.png" alt="Clocks menu" width="200"></p>

| Application | What it does | Screenshot |
| --- | --- | --- |
| **Binary Clock** | Encodes the current hour, minute, and second as binary columns. | <img src="docs/gallery/png/clocks/binary-clock/default/light.png" alt="Binary Clock" width="200"> |
| **Unix Time** | Displays the current Unix epoch timestamp. | <img src="docs/gallery/png/clocks/unix-time/default/light.png" alt="Unix Time" width="200"> |
| **UTC Clock** | Shows the current Coordinated Universal Time. | <img src="docs/gallery/png/clocks/utc-clock/default/light.png" alt="UTC Clock" width="200"> |
| **ISO Week** | Displays the ISO week number and ISO weekday context. | <img src="docs/gallery/png/clocks/iso-week/default/light.png" alt="ISO Week" width="200"> |
| **Day of Year** | Shows the ordinal day and remaining days in the year. | <img src="docs/gallery/png/clocks/day-of-year/default/light.png" alt="Day of Year" width="200"> |
| **Calendar** | Renders a complete month calendar with the current day highlighted. | <img src="docs/gallery/png/clocks/month-calendar/default/light.png" alt="Calendar" width="200"> |
| **World Clocks** | Compares the current time across several world cities. | <img src="docs/gallery/png/clocks/world-clocks/default/light.png" alt="World Clocks" width="200"> |
| **Local + UTC** | Places local time and UTC side by side. | <img src="docs/gallery/png/clocks/local-utc/default/light.png" alt="Local and UTC" width="200"> |
| **Internet Beats** | Converts the day into Swatch Internet Time beats. | <img src="docs/gallery/png/clocks/internet-beats/default/light.png" alt="Internet Beats" width="200"> |
| **Decimal Time** | Represents the current day using decimal hours and subdivisions. | <img src="docs/gallery/png/clocks/decimal-time/default/light.png" alt="Decimal Time" width="200"> |
| **Julian Day** | Displays the astronomical Julian Date. | <img src="docs/gallery/png/clocks/julian-day/default/light.png" alt="Julian Day" width="200"> |
| **Time Progress** | Visualizes progress through the day, month, and year. | <img src="docs/gallery/png/clocks/time-progress/default/light.png" alt="Time Progress" width="200"> |

## Time Tools

Timers, alarms, interval pacing, and rhythm tools.

<p align="center"><img src="docs/gallery/png/os/menu/time-tools/light.png" alt="Time Tools menu" width="200"></p>

| Application | What it does | Screenshot |
| --- | --- | --- |
| **Stopwatch** | Measures elapsed time and records lap information. | <img src="docs/gallery/png/time-tools/stopwatch/running/light.png" alt="Stopwatch" width="200"> |
| **Countdown** | Runs an adjustable countdown timer. | <img src="docs/gallery/png/time-tools/countdown/running/light.png" alt="Countdown" width="200"> |
| **Daily Alarm** | Configures a recurring local-time vibration alarm. | <img src="docs/gallery/png/time-tools/daily-alarm/enabled/light.png" alt="Daily Alarm" width="200"> |
| **Pomodoro** | Alternates focused work and break countdowns. | <img src="docs/gallery/png/time-tools/pomodoro/focus/light.png" alt="Pomodoro" width="200"> |
| **Intervals** | Alternates configurable work and rest periods. | <img src="docs/gallery/png/time-tools/intervals/work/light.png" alt="Intervals" width="200"> |
| **Metronome** | Provides an adjustable beat and haptic tempo reference. | <img src="docs/gallery/png/time-tools/metronome/running/light.png" alt="Metronome" width="200"> |

## Sensors

Battery, motion, activity, health, and runtime diagnostics from Watchy's
on-board hardware.

<p align="center"><img src="docs/gallery/png/os/menu/sensors/light.png" alt="Sensors menu" width="200"></p>

| Application | What it does | Screenshot |
| --- | --- | --- |
| **Battery Gauge** | Shows calibrated voltage, percentage, and remaining capacity. | <img src="docs/gallery/png/sensors/battery-gauge/default/light.png" alt="Battery Gauge" width="200"> |
| **Power Budget** | Estimates remaining energy and runtime from the battery model. | <img src="docs/gallery/png/sensors/power-budget/default/light.png" alt="Power Budget" width="200"> |
| **Charge Status** | Reports USB presence, charging state, voltage, and charge level. | <img src="docs/gallery/png/sensors/charge-status/charging/light.png" alt="Charge Status" width="200"> |
| **BMA Temperature** | Displays the accelerometer's internal temperature reading. | <img src="docs/gallery/png/sensors/bma-temperature/default/light.png" alt="BMA Temperature" width="200"> |
| **Raw Accel** | Shows raw X, Y, and Z acceleration samples. | <img src="docs/gallery/png/sensors/raw-accel/default/light.png" alt="Raw Accel" width="200"> |
| **G Force** | Converts the acceleration vector into magnitude measured in g. | <img src="docs/gallery/png/sensors/g-force/default/light.png" alt="G Force" width="200"> |
| **Spirit Level** | Uses acceleration to provide a two-axis level indicator. | <img src="docs/gallery/png/sensors/spirit-level/default/light.png" alt="Spirit Level" width="200"> |
| **Orientation** | Classifies the current face-up, face-down, or side orientation. | <img src="docs/gallery/png/sensors/orientation/face-up/light.png" alt="Orientation" width="200"> |
| **Motion Score** | Summarizes recent movement intensity as a compact score. | <img src="docs/gallery/png/sensors/motion-score/moving/light.png" alt="Motion Score" width="200"> |
| **Step Counter** | Displays the BMA423 hardware step count. | <img src="docs/gallery/png/sensors/step-counter/default/light.png" alt="Step Counter" width="200"> |
| **Step Goal** | Shows progress toward the configured daily step target. | <img src="docs/gallery/png/sensors/step-goal/default/light.png" alt="Step Goal" width="200"> |
| **Walk Distance** | Estimates distance traveled from step count. | <img src="docs/gallery/png/sensors/walk-distance/default/light.png" alt="Walk Distance" width="200"> |
| **Step Calories** | Estimates activity calories from step count. | <img src="docs/gallery/png/sensors/step-calories/default/light.png" alt="Step Calories" width="200"> |
| **Activity State** | Reports the BMA423 activity classifier state. | <img src="docs/gallery/png/sensors/activity/walking/light.png" alt="Activity State" width="200"> |
| **Sensor Status** | Summarizes accelerometer enablement, status, errors, and sensor time. | <img src="docs/gallery/png/sensors/sensor-status/healthy/light.png" alt="Sensor Status" width="200"> |
| **Uptime** | Shows elapsed time since the last cold boot. | <img src="docs/gallery/png/sensors/uptime/default/light.png" alt="Uptime" width="200"> |
| **Shake Counter** | Counts deliberate shakes detected by the accelerometer. | <img src="docs/gallery/png/sensors/shake-counter/default/light.png" alt="Shake Counter" width="200"> |

## Bluetooth

BLE discovery, signal analysis, advertisement inspection, and configurable
Watchy beacon modes.

<p align="center"><img src="docs/gallery/png/os/menu/bluetooth/light.png" alt="Bluetooth menu" width="200"></p>

| Application | What it does | Screenshot |
| --- | --- | --- |
| **BLE Scanner** | Lists nearby Bluetooth Low Energy devices and signal strength. | <img src="docs/gallery/png/bluetooth/ble-scanner/result/light.png" alt="BLE Scanner" width="200"> |
| **Device Count** | Summarizes discovered, named, service-bearing, and manufacturer-bearing devices. | <img src="docs/gallery/png/bluetooth/device-count/result/light.png" alt="Device Count" width="200"> |
| **Strongest Signal** | Highlights the nearest or strongest observed BLE advertiser. | <img src="docs/gallery/png/bluetooth/strongest-signal/result/light.png" alt="Strongest Signal" width="200"> |
| **Named Devices** | Filters scan results to advertisers with readable device names. | <img src="docs/gallery/png/bluetooth/named-devices/result/light.png" alt="Named Devices" width="200"> |
| **Service UUIDs** | Lists advertised Bluetooth service UUIDs. | <img src="docs/gallery/png/bluetooth/service-uuids/result/light.png" alt="Service UUIDs" width="200"> |
| **Manufacturer IDs** | Lists manufacturer identifiers found in advertisement data. | <img src="docs/gallery/png/bluetooth/manufacturer-ids/result/light.png" alt="Manufacturer IDs" width="200"> |
| **RSSI Bands** | Groups nearby devices into close, near, and far signal bands. | <img src="docs/gallery/png/bluetooth/rssi-bands/result/light.png" alt="RSSI Bands" width="200"> |
| **BLE Addresses** | Lists observed BLE addresses and RSSI values. | <img src="docs/gallery/png/bluetooth/ble-addresses/result/light.png" alt="BLE Addresses" width="200"> |
| **BLE Radar** | Visualizes nearby devices as comparative signal-strength bars. | <img src="docs/gallery/png/bluetooth/ble-radar/result/light.png" alt="BLE Radar" width="200"> |
| **TX Power Survey** | Summarizes advertised transmit-power values. | <img src="docs/gallery/png/bluetooth/tx-power/result/light.png" alt="TX Power Survey" width="200"> |
| **iBeacon Watch** | Counts nearby iBeacon and Apple manufacturer packets. | <img src="docs/gallery/png/bluetooth/ibeacon-watch/result/light.png" alt="iBeacon Watch" width="200"> |
| **Watchy Beacon** | Advertises a generic Watchy identity over BLE. | <img src="docs/gallery/png/bluetooth/watchy-beacon/on-air/light.png" alt="Watchy Beacon" width="200"> |
| **Battery Beacon** | Advertises the standard BLE battery service. | <img src="docs/gallery/png/bluetooth/battery-beacon/on-air/light.png" alt="Battery Beacon" width="200"> |
| **Time Beacon** | Advertises deterministic time data in a service payload. | <img src="docs/gallery/png/bluetooth/time-beacon/on-air/light.png" alt="Time Beacon" width="200"> |
| **Step Beacon** | Advertises the current step count through a custom service. | <img src="docs/gallery/png/bluetooth/step-beacon/on-air/light.png" alt="Step Beacon" width="200"> |
| **Name Badge** | Advertises a configured readable device name. | <img src="docs/gallery/png/bluetooth/name-badge/on-air/light.png" alt="Name Badge" width="200"> |

## Build And Install

This project targets Watchy v3 / ESP32-S3 and uses PlatformIO:

```powershell
pio run -e esp32-s3-devkitc-1
pio run -e esp32-s3-devkitc-1 -t upload --upload-port COM3
```

Change `COM3` to the port assigned to your Watchy. The upstream hardware setup
guide is available at [watchy.sqfmi.com/docs/getting-started](https://watchy.sqfmi.com/docs/getting-started).

## Application SDK

Applications share standard screens, lists, value editors, semantic input,
feedback, themes, and durable settings. See the [Watchy Application SDK](docs/WatchySDK.md)
and the [Standard Counter example](examples/Apps/StandardCounter/StandardCounter.cpp).

## Rebuild The Gallery

The isolated gallery firmware captures every renderer without reading private
settings or activating the panel, radios, sensors, vibration, storage, or
sleep. The receiver rejects incomplete, reordered, duplicate, malformed, or
CRC-invalid frames.

```powershell
python -m pip install -r tools/requirements-gallery.txt
pio run -e gallery -t upload --upload-port COM3
python tools/capture_gallery.py --port COM3 --output docs/gallery --replace
pio run -e esp32-s3-devkitc-1 -t upload --upload-port COM3
```

See the [deterministic gallery documentation](docs/DeterministicGallery.md)
and its machine-readable [capture manifest](docs/gallery/manifest.json).

## Project Links

- [Watchy documentation](https://watchy.sqfmi.com/docs/getting-started)
- [Watchy case and accessories](https://shop.sqfmi.com)
- [Community Discord](https://discord.gg/ZXDegGV8E7)
- [Contributing](CONTRIBUTING.md)
- [License](LICENSE)


