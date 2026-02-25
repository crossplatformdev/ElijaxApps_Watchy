# Watchy Application Suite

A complete application-focused firmware for the Watchy open-source e-paper
watch. It provides **142 applications across 10 task-oriented menu
categories**, a consistent Watchy OS interface, deterministic public
screenshots, and a lightweight SDK for adding more applications.

Start with the [documentation index](docs/readme.md) for firmware architecture,
gallery capture, calibration studies, timing validation, and measured build
results.

The one-to-five screenshots for each app come directly from the real
`200x200`, 1-bit firmware framebuffer. They are not UI mockups. Public-safe
fixtures provide the fixed time, sensor readings, health profile, locations,
and network results shown in the gallery; normal firmware uses live hardware
and configured data.

The SDK calibration scenes expose the spatial-dither ramp, semantic tones,
widget states, dialogs, lists, disabled controls, and graph rendering used on
the 1-bit panel.

| Gray8 ramp | Light tones | Dark tones |
| --- | --- | --- |
| ![Gray8 calibration ramp](docs/gallery/png/sdk/grayscale/ramp/light.png) | ![Gray8 semantic tones light](docs/gallery/png/sdk/grayscale/semantic-tones/light.png) | ![Gray8 semantic tones dark](docs/gallery/png/sdk/grayscale/semantic-tones/dark.png) |
| ![Gray8 widgets light](docs/gallery/png/sdk/grayscale/widgets/light.png) | ![Gray8 widgets dark](docs/gallery/png/sdk/grayscale/widgets/dark.png) | ![Gray8 dialog](docs/gallery/png/sdk/grayscale/dialog/light.png) |
| ![Gray8 list](docs/gallery/png/sdk/grayscale/list/light.png) | ![Gray8 disabled controls](docs/gallery/png/sdk/grayscale/disabled-controls/light.png) | ![Gray8 graph](docs/gallery/png/sdk/grayscale/graph/light.png) |

Every WatchFace also has an independent pixel-golden baseline in light and
dark polarity. These images use fixed time, weather, steps, battery, and BPM
fixtures; normal firmware continues to read live hardware.

| WatchFace | Light | Dark |
| --- | --- | --- |
| 7-SEG | ![7-SEG light](docs/gallery/png/watchfaces/7-seg/light.png) | ![7-SEG dark](docs/gallery/png/watchfaces/7-seg/dark.png) |
| Basic | ![Basic light](docs/gallery/png/watchfaces/basic/light.png) | ![Basic dark](docs/gallery/png/watchfaces/basic/dark.png) |
| DOS | ![DOS light](docs/gallery/png/watchfaces/dos/light.png) | ![DOS dark](docs/gallery/png/watchfaces/dos/dark.png) |
| MacPaint | ![MacPaint light](docs/gallery/png/watchfaces/macpaint/light.png) | ![MacPaint dark](docs/gallery/png/watchfaces/macpaint/dark.png) |
| Mario | ![Mario light](docs/gallery/png/watchfaces/mario/light.png) | ![Mario dark](docs/gallery/png/watchfaces/mario/dark.png) |
| Pokemon | ![Pokemon light](docs/gallery/png/watchfaces/pokemon/light.png) | ![Pokemon dark](docs/gallery/png/watchfaces/pokemon/dark.png) |
| Starry Horizon | ![Starry Horizon light](docs/gallery/png/watchfaces/starry-horizon/light.png) | ![Starry Horizon dark](docs/gallery/png/watchfaces/starry-horizon/dark.png) |
| Tetris | ![Tetris light](docs/gallery/png/watchfaces/tetris/light.png) | ![Tetris dark](docs/gallery/png/watchfaces/tetris/dark.png) |

## Watchy OS

Watchy OS presents every feature through a two-level category menu with
consistent typography, selection, input, themes, and display lifecycle. The
root view provides access to all ten application groups.

![Watchy OS category menu](docs/gallery/png/os/menu/categories/light.png)

| Menu category | Applications |
| --- | ---: |
| Clocks & Sky | 16 |
| Timers & Focus | 6 |
| Health & Care | 15 |
| Safety & Aid | 20 |
| Sensors & Steps | 18 |
| Everyday Tools | 13 |
| Games & Puzzles | 15 |
| Network & Web | 9 |
| Bluetooth | 16 |
| Watch & System | 14 |
| **Total** | **142** |

| | |
| --- | --- |
| ![Clocks and Sky menu](docs/gallery/png/os/menu/clocks-sky/light.png) | ![Timers and Focus menu](docs/gallery/png/os/menu/timers-focus/light.png) |
| ![Health and Care menu](docs/gallery/png/os/menu/health-wellness/light.png) | ![Safety and Aid menu](docs/gallery/png/os/menu/safety-first-aid/light.png) |
| ![Sensors and Steps menu](docs/gallery/png/os/menu/sensors-activity/light.png) | ![Everyday Tools menu](docs/gallery/png/os/menu/everyday-tools/light.png) |
| ![Games and Puzzles menu](docs/gallery/png/os/menu/games-puzzles/light.png) | ![Network and Web menu](docs/gallery/png/os/menu/network-web/light.png) |
| ![Bluetooth menu](docs/gallery/png/os/menu/bluetooth/light.png) | ![Watch and System menu](docs/gallery/png/os/menu/watch-system/light.png) |

The on-watch order puts everyday time, health, safety, and sensor workflows
first. The gallery below remains grouped by stable implementation family:
[System](#system) · [Utilities](#utilities) · [Networking](#networking) ·
[Astronomy](#astronomy) · [Healthcare](#healthcare) · [Games](#games) ·
[Clocks](#clocks) · [Time Tools](#time-tools) · [Sensors](#sensors) ·
[Bluetooth](#bluetooth).

## System

Core Watchy OS configuration, identity, connectivity, watch-face selection,
and appearance controls.

| Application | What it does | Screenshot 1 | Screenshot 2 | Screenshot 3 | Screenshot 4 | Screenshot 5 |
| --- | --- | --- | --- | --- | --- | --- |
| **About Watchy** | Displays firmware, hardware, and project information. | ![About Watchy - default](docs/gallery/png/system/about-watchy/default/light.png) |  |  |  |  |
| **Set Time** | Provides an on-watch editor for the local date and time. | ![Set Time - default](docs/gallery/png/system/set-time/default/light.png) |  |  |  |  |
| **Setup WiFi** | Configures Wi-Fi and reports the resulting connection state. | ![Setup WiFi - connecting](docs/gallery/png/system/setup-wifi/connecting/light.png) | ![Setup WiFi - connected](docs/gallery/png/system/setup-wifi/connected/light.png) | ![Setup WiFi - cancelled](docs/gallery/png/system/setup-wifi/cancelled/light.png) | ![Setup WiFi - timed out](docs/gallery/png/system/setup-wifi/timed-out/light.png) | ![Setup WiFi - failed](docs/gallery/png/system/setup-wifi/failed/light.png) |
| **Watch Faces** | Selects the active watch face from the installed collection. | ![Watch Faces - selector](docs/gallery/png/system/watch-faces/selector/light.png) |  |  |  |  |
| **Theme Colours** | Switches the shared Watchy OS interface between light and dark themes. | ![Theme Colours - light selected](docs/gallery/png/system/theme-colours/light-selected/light.png) | ![Theme Colours - dark selected](docs/gallery/png/system/theme-colours/dark-selected/light.png) |  |  |  |

## Utilities

Everyday randomizers, diagnostics, hardware tools, generators, and unit
converters.

| Application | What it does | Screenshot 1 | Screenshot 2 | Screenshot 3 | Screenshot 4 | Screenshot 5 |
| --- | --- | --- | --- | --- | --- | --- |
| **Vibrate Motor** | Runs a short haptic motor test from the Utilities menu. | ![Vibrate Motor - default](docs/gallery/png/utilities/vibrate-motor/default/light.png) |  |  |  |  |
| **Accelerometer** | Shows the current three-axis accelerometer reading and orientation. | ![Accelerometer - face up](docs/gallery/png/utilities/accelerometer/face-up/light.png) | ![Accelerometer - face down](docs/gallery/png/utilities/accelerometer/face-down/light.png) | ![Accelerometer - error](docs/gallery/png/utilities/accelerometer/error/light.png) |  |  |
| **Sync NTP** | Synchronizes the RTC with an internet time source and reports success or failure. | ![Sync NTP - loading](docs/gallery/png/utilities/sync-ntp/loading/light.png) | ![Sync NTP - success](docs/gallery/png/utilities/sync-ntp/success/light.png) | ![Sync NTP - failure](docs/gallery/png/utilities/sync-ntp/failure/light.png) |  |  |
| **Coin Flip** | Produces a quick heads-or-tails decision. | ![Coin Flip - heads](docs/gallery/png/utilities/coin-flip/heads/light.png) | ![Coin Flip - tails](docs/gallery/png/utilities/coin-flip/tails/light.png) |  |  |  |
| **D6 Dice** | Rolls a standard six-sided die. | ![D6 Dice - result](docs/gallery/png/utilities/d6-dice/result/light.png) |  |  |  |  |
| **D20 Dice** | Rolls a twenty-sided tabletop die. | ![D20 Dice - result](docs/gallery/png/utilities/d20-dice/result/light.png) |  |  |  |  |
| **Random Number** | Generates a reusable random numeric value. | ![Random Number - result](docs/gallery/png/utilities/random-number/result/light.png) |  |  |  |  |
| **Decision Maker** | Answers a simple yes-or-no choice. | ![Decision Maker - yes](docs/gallery/png/utilities/decision-maker/yes/light.png) | ![Decision Maker - no](docs/gallery/png/utilities/decision-maker/no/light.png) |  |  |  |
| **Password Generator** | Creates a compact mixed-character password on the watch. | ![Password Generator - result](docs/gallery/png/utilities/password-generator/result/light.png) |  |  |  |  |
| **UUID Generator** | Creates and displays a standards-shaped UUID. | ![UUID Generator - result](docs/gallery/png/utilities/uuid-generator/result/light.png) |  |  |  |  |
| **I2C Scanner** | Probes the I2C bus and lists responding device addresses. | ![I2C Scanner - found](docs/gallery/png/utilities/i2c-scanner/found/light.png) | ![I2C Scanner - empty](docs/gallery/png/utilities/i2c-scanner/empty/light.png) |  |  |  |
| **Chip Info** | Summarizes the ESP32-S3, CPU, flash, and SDK configuration. | ![Chip Info - default](docs/gallery/png/utilities/chip-info/default/light.png) |  |  |  |  |
| **Heap Monitor** | Reports free heap, minimum free heap, and largest available block. | ![Heap Monitor - default](docs/gallery/png/utilities/heap-monitor/default/light.png) |  |  |  |  |
| **Wake Reason** | Explains which wake source resumed the watch. | ![Wake Reason - timer](docs/gallery/png/utilities/wake-reason/timer/light.png) | ![Wake Reason - button](docs/gallery/png/utilities/wake-reason/button/light.png) |  |  |  |
| **Reset Reason** | Reports the ESP32 reset cause and raw reason code. | ![Reset Reason - power on](docs/gallery/png/utilities/reset-reason/power-on/light.png) | ![Reset Reason - panic](docs/gallery/png/utilities/reset-reason/panic/light.png) |  |  |  |
| **Button Tester** | Counts input events so every physical button can be checked. | ![Button Tester - idle](docs/gallery/png/utilities/button-tester/idle/light.png) | ![Button Tester - pressed](docs/gallery/png/utilities/button-tester/pressed/light.png) |  |  |  |
| **Vibration Lab** | Selects and previews repeatable haptic patterns. | ![Vibration Lab - tap](docs/gallery/png/utilities/vibration-lab/tap/light.png) | ![Vibration Lab - heartbeat](docs/gallery/png/utilities/vibration-lab/heartbeat/light.png) |  |  |  |
| **Screen Ruler** | Draws a calibrated on-screen ruler for quick measurements. | ![Screen Ruler - default](docs/gallery/png/utilities/screen-ruler/default/light.png) |  |  |  |  |
| **Temperature Converter** | Converts between Celsius and Fahrenheit. | ![Temperature Converter - default](docs/gallery/png/utilities/temperature-converter/default/light.png) |  |  |  |  |
| **Length Converter** | Converts common metric and imperial lengths. | ![Length Converter - default](docs/gallery/png/utilities/length-converter/default/light.png) |  |  |  |  |
| **Weight Converter** | Converts common metric and imperial weights. | ![Weight Converter - default](docs/gallery/png/utilities/weight-converter/default/light.png) |  |  |  |  |
| **Base Converter** | Displays a number in decimal, hexadecimal, octal, and binary. | ![Base Converter - default](docs/gallery/png/utilities/base-converter/default/light.png) |  |  |  |  |
| **Pace Converter** | Converts running pace between common distance units. | ![Pace Converter - default](docs/gallery/png/utilities/pace-converter/default/light.png) |  |  |  |  |

## Networking

Compact network inspection and text-retrieval tools. Scanning tools must only
be used on networks and hosts you are authorized to test.

| Application | What it does | Screenshot 1 | Screenshot 2 | Screenshot 3 | Screenshot 4 | Screenshot 5 |
| --- | --- | --- | --- | --- | --- | --- |
| **Browser** | Fetches and presents a compact text-oriented web page. | ![Browser - editor](docs/gallery/png/networking/browser/editor/light.png) | ![Browser - loading](docs/gallery/png/networking/browser/loading/light.png) | ![Browser - result](docs/gallery/png/networking/browser/result/light.png) | ![Browser - error](docs/gallery/png/networking/browser/error/light.png) |  |
| **RSS Feed** | Retrieves a feed and displays recent item titles and summaries. | ![RSS Feed - source](docs/gallery/png/networking/rss-feed/source/light.png) | ![RSS Feed - custom editor](docs/gallery/png/networking/rss-feed/custom-editor/light.png) | ![RSS Feed - loading](docs/gallery/png/networking/rss-feed/loading/light.png) | ![RSS Feed - headlines](docs/gallery/png/networking/rss-feed/headlines/light.png) | ![RSS Feed - article](docs/gallery/png/networking/rss-feed/article/light.png) |
| **Ping** | Measures reachability, replies, and round-trip latency. | ![Ping - editor](docs/gallery/png/networking/ping/editor/light.png) | ![Ping - loading](docs/gallery/png/networking/ping/loading/light.png) | ![Ping - result](docs/gallery/png/networking/ping/result/light.png) | ![Ping - error](docs/gallery/png/networking/ping/error/light.png) |  |
| **Traceroute** | Lists the network hops observed on the path to a host. | ![Traceroute - editor](docs/gallery/png/networking/traceroute/editor/light.png) | ![Traceroute - loading](docs/gallery/png/networking/traceroute/loading/light.png) | ![Traceroute - result](docs/gallery/png/networking/traceroute/result/light.png) | ![Traceroute - cancelled](docs/gallery/png/networking/traceroute/cancelled/light.png) |  |
| **Port Scanner** | Checks a focused set of common TCP ports on an authorized host. | ![Port Scanner - editor](docs/gallery/png/networking/port-scanner/editor/light.png) | ![Port Scanner - loading](docs/gallery/png/networking/port-scanner/loading/light.png) | ![Port Scanner - result](docs/gallery/png/networking/port-scanner/result/light.png) | ![Port Scanner - empty](docs/gallery/png/networking/port-scanner/empty/light.png) |  |
| **DNS Query** | Resolves a host name and displays the DNS result and status. | ![DNS Query - editor](docs/gallery/png/networking/dns-query/editor/light.png) | ![DNS Query - loading](docs/gallery/png/networking/dns-query/loading/light.png) | ![DNS Query - result](docs/gallery/png/networking/dns-query/result/light.png) | ![DNS Query - error](docs/gallery/png/networking/dns-query/error/light.png) |  |
| **Reverse DNS** | Resolves an IP address back to its PTR host name. | ![Reverse DNS - editor](docs/gallery/png/networking/reverse-dns/editor/light.png) | ![Reverse DNS - loading](docs/gallery/png/networking/reverse-dns/loading/light.png) | ![Reverse DNS - result](docs/gallery/png/networking/reverse-dns/result/light.png) | ![Reverse DNS - error](docs/gallery/png/networking/reverse-dns/error/light.png) |  |
| **DuckDuckGo** | Runs a lightweight web search and lists compact results. | ![DuckDuckGo - editor](docs/gallery/png/networking/duckduckgo/editor/light.png) | ![DuckDuckGo - loading](docs/gallery/png/networking/duckduckgo/loading/light.png) | ![DuckDuckGo - result](docs/gallery/png/networking/duckduckgo/result/light.png) | ![DuckDuckGo - empty](docs/gallery/png/networking/duckduckgo/empty/light.png) |  |
| **WiFi Survey** | Lists nearby access points with channel, RSSI, security, and BSSID. | ![WiFi Survey - scanning](docs/gallery/png/networking/wifi-survey/scanning/light.png) | ![WiFi Survey - result](docs/gallery/png/networking/wifi-survey/result/light.png) | ![WiFi Survey - empty](docs/gallery/png/networking/wifi-survey/empty/light.png) |  |  |

## Astronomy

Daily solar, lunar, and tidal information using configured locations and
stations.

| Application | What it does | Screenshot 1 | Screenshot 2 | Screenshot 3 | Screenshot 4 | Screenshot 5 |
| --- | --- | --- | --- | --- | --- | --- |
| **Sun Rise** | Calculates sunrise, sunset, and daylight information for a location. | ![Sun Rise - result](docs/gallery/png/astronomy/sun-rise/result/light.png) | ![Sun Rise - no events](docs/gallery/png/astronomy/sun-rise/no-events/light.png) |  |  |  |
| **Moon Rise** | Calculates moonrise and moonset times for a location. | ![Moon Rise - result](docs/gallery/png/astronomy/moon-rise/result/light.png) | ![Moon Rise - no events](docs/gallery/png/astronomy/moon-rise/no-events/light.png) |  |  |  |
| **Moon Phase** | Shows the current lunar phase, age, and illumination. | ![Moon Phase - default](docs/gallery/png/astronomy/moon-phase/default/light.png) |  |  |  |  |
| **Tides** | Displays four daily tide events, coefficients, range, and station data. | ![Tides - result](docs/gallery/png/astronomy/tides/result/light.png) | ![Tides - unavailable](docs/gallery/png/astronomy/tides/unavailable/light.png) |  |  |  |

## Healthcare

Medical identity, safety monitoring, reminders, emergency references, and
guided wellbeing tools. These features are informational aids and are not a
substitute for professional medical care or emergency services.

| Application | What it does | Screenshot 1 | Screenshot 2 | Screenshot 3 | Screenshot 4 | Screenshot 5 |
| --- | --- | --- | --- | --- | --- | --- |
| **Heart Rate** | Estimates pulse using experimental accelerometer-based ballistocardiography. | ![Heart Rate - ready](docs/gallery/png/healthcare/heart-rate/ready/light.png) | ![Heart Rate - measuring](docs/gallery/png/healthcare/heart-rate/measuring/light.png) | ![Heart Rate - result](docs/gallery/png/healthcare/heart-rate/result/light.png) | ![Heart Rate - invalid](docs/gallery/png/healthcare/heart-rate/invalid/light.png) |  |
| **UN Dog Plate** | Formats emergency identity and medical details in a compact plate-style view. | ![UN Dog Plate - demo](docs/gallery/png/healthcare/un-dog-plate/demo/light.png) |  |  |  |  |
| **Medical ID** | Presents a concise emergency summary from the configured medical profile. | ![Medical ID - demo](docs/gallery/png/healthcare/medical-id/demo/light.png) | ![Medical ID - empty](docs/gallery/png/healthcare/medical-id/empty/light.png) |  |  |  |
| **ICE Contact** | Displays the configured in-case-of-emergency contact. | ![ICE Contact - demo](docs/gallery/png/healthcare/ice-contact/demo/light.png) | ![ICE Contact - empty](docs/gallery/png/healthcare/ice-contact/empty/light.png) |  |  |  |
| **Blood Type** | Shows the configured blood group prominently for emergency reference. | ![Blood Type - demo](docs/gallery/png/healthcare/blood-type/demo/light.png) | ![Blood Type - empty](docs/gallery/png/healthcare/blood-type/empty/light.png) |  |  |  |
| **Allergies** | Lists important configured allergies. | ![Allergies - demo](docs/gallery/png/healthcare/allergies/demo/light.png) | ![Allergies - empty](docs/gallery/png/healthcare/allergies/empty/light.png) |  |  |  |
| **Medications** | Lists current configured medications. | ![Medications - demo](docs/gallery/png/healthcare/medications/demo/light.png) | ![Medications - empty](docs/gallery/png/healthcare/medications/empty/light.png) |  |  |  |
| **Conditions** | Lists relevant configured medical conditions. | ![Conditions - demo](docs/gallery/png/healthcare/conditions/demo/light.png) | ![Conditions - empty](docs/gallery/png/healthcare/conditions/empty/light.png) |  |  |  |
| **Edit Medical ID** | Provides an on-watch editor for the emergency medical profile. | ![Edit Medical ID - field list](docs/gallery/png/healthcare/edit-medical-id/field-list/light.png) | ![Edit Medical ID - editor](docs/gallery/png/healthcare/edit-medical-id/editor/light.png) |  |  |  |
| **Fall Detector** | Runs live detection or records bounded background candidate traces for calibration. | ![Fall Detector - monitoring](docs/gallery/png/healthcare/fall-detector/monitoring/light.png) | ![Fall Detector - low g](docs/gallery/png/healthcare/fall-detector/low-g/light.png) | ![Fall Detector - checking](docs/gallery/png/healthcare/fall-detector/checking/light.png) | ![Fall Detector - alert](docs/gallery/png/healthcare/fall-detector/alert/light.png) | ![Fall Detector - configuration](docs/gallery/png/healthcare/fall-detector/configuration/light.png) |
| **Body Position** | Reports body or watch orientation for quick safety assessment. | ![Body Position - face up](docs/gallery/png/healthcare/body-position/face-up/light.png) | ![Body Position - face down](docs/gallery/png/healthcare/body-position/face-down/light.png) | ![Body Position - edge](docs/gallery/png/healthcare/body-position/edge/light.png) |  |  |
| **Saved Location** | Displays the configured emergency location details. | ![Saved Location - coordinates](docs/gallery/png/healthcare/saved-location/coordinates/light.png) | ![Saved Location - city](docs/gallery/png/healthcare/saved-location/city/light.png) | ![Saved Location - empty](docs/gallery/png/healthcare/saved-location/empty/light.png) |  |  |
| **SOS Screen** | Shows a high-contrast emergency message and essential identity details. | ![SOS Screen - demo](docs/gallery/png/healthcare/sos-screen/demo/light.png) |  |  |  |  |
| **SOS BLE Beacon** | Broadcasts a configured emergency payload over Bluetooth Low Energy. | ![SOS BLE Beacon - broadcasting](docs/gallery/png/healthcare/sos-ble/broadcasting/light.png) | ![SOS BLE Beacon - paused](docs/gallery/png/healthcare/sos-ble/paused/light.png) |  |  |  |
| **Check-In Timer** | Starts a safety timer that expects the wearer to acknowledge before expiry. | ![Check-In Timer - ready](docs/gallery/png/healthcare/check-in-timer/ready/light.png) | ![Check-In Timer - armed](docs/gallery/png/healthcare/check-in-timer/armed/light.png) | ![Check-In Timer - expired](docs/gallery/png/healthcare/check-in-timer/expired/light.png) |  |  |
| **Medication Alert** | Enables and displays a recurring medication reminder. | ![Medication Alert - disabled](docs/gallery/png/healthcare/medication-alert/disabled/light.png) | ![Medication Alert - enabled](docs/gallery/png/healthcare/medication-alert/enabled/light.png) |  |  |  |
| **Hydration Alert** | Enables and displays a recurring hydration reminder. | ![Hydration Alert - disabled](docs/gallery/png/healthcare/hydration-alert/disabled/light.png) | ![Hydration Alert - enabled](docs/gallery/png/healthcare/hydration-alert/enabled/light.png) |  |  |  |
| **Breathing Coach** | Guides paced inhale and exhale intervals. | ![Breathing Coach - inhale](docs/gallery/png/healthcare/breathing-coach/inhale/light.png) | ![Breathing Coach - hold](docs/gallery/png/healthcare/breathing-coach/hold/light.png) | ![Breathing Coach - exhale](docs/gallery/png/healthcare/breathing-coach/exhale/light.png) | ![Breathing Coach - paused](docs/gallery/png/healthcare/breathing-coach/paused/light.png) |  |
| **CPR Metronome** | Provides a steady compression cadence reference for CPR. | ![CPR Metronome - ready](docs/gallery/png/healthcare/cpr-metronome/ready/light.png) | ![CPR Metronome - running](docs/gallery/png/healthcare/cpr-metronome/running/light.png) |  |  |  |
| **Recovery Position** | Presents concise recovery-position guidance. | ![Recovery Position - default](docs/gallery/png/healthcare/recovery-position/default/light.png) |  |  |  |  |
| **Stroke FAST** | Presents the Face, Arms, Speech, Time stroke-recognition checklist. | ![Stroke FAST - default](docs/gallery/png/healthcare/stroke-fast/default/light.png) |  |  |  |  |
| **Choking Response** | Shows concise first-aid steps for a choking emergency. | ![Choking Response - default](docs/gallery/png/healthcare/choking-response/default/light.png) |  |  |  |  |
| **Seizure Aid** | Shows immediate safety guidance for assisting during a seizure. | ![Seizure Aid - default](docs/gallery/png/healthcare/seizure-aid/default/light.png) |  |  |  |  |
| **Severe Bleeding** | Presents first-aid actions for controlling severe bleeding. | ![Severe Bleeding - default](docs/gallery/png/healthcare/severe-bleeding/default/light.png) |  |  |  |  |
| **Burn First Aid** | Presents immediate first-aid guidance for burns. | ![Burn First Aid - default](docs/gallery/png/healthcare/burn-first-aid/default/light.png) |  |  |  |  |
| **Heat Emergency** | Lists recognition and response steps for heat illness. | ![Heat Emergency - default](docs/gallery/png/healthcare/heat-emergency/default/light.png) |  |  |  |  |
| **Hypothermia** | Lists recognition and response steps for dangerous cold exposure. | ![Hypothermia - default](docs/gallery/png/healthcare/hypothermia/default/light.png) |  |  |  |  |
| **Poisoning** | Presents immediate poisoning response guidance and cautions. | ![Poisoning - default](docs/gallery/png/healthcare/poisoning/default/light.png) |  |  |  |  |
| **Anaphylaxis** | Presents urgent response guidance for severe allergic reactions. | ![Anaphylaxis - default](docs/gallery/png/healthcare/anaphylaxis/default/light.png) |  |  |  |  |
| **Opioid Overdose** | Presents recognition and naloxone-oriented emergency guidance. | ![Opioid Overdose - default](docs/gallery/png/healthcare/opioid-overdose/default/light.png) |  |  |  |  |
| **Asthma Attack** | Presents immediate response guidance for an asthma attack. | ![Asthma Attack - default](docs/gallery/png/healthcare/asthma-attack/default/light.png) |  |  |  |  |
| **Emergency Numbers** | Keeps important emergency telephone numbers available offline. | ![Emergency Numbers - default](docs/gallery/png/healthcare/emergency-numbers/default/light.png) |  |  |  |  |
| **Pain Log** | Records a quick pain-intensity entry for later reference. | ![Pain Log - empty](docs/gallery/png/healthcare/pain-log/empty/light.png) | ![Pain Log - editing](docs/gallery/png/healthcare/pain-log/editing/light.png) | ![Pain Log - saved](docs/gallery/png/healthcare/pain-log/saved/light.png) |  |  |
| **Symptom Note** | Stores a compact symptom note and timestamp. | ![Symptom Note - display](docs/gallery/png/healthcare/symptom-note/display/light.png) | ![Symptom Note - empty](docs/gallery/png/healthcare/symptom-note/empty/light.png) | ![Symptom Note - editor](docs/gallery/png/healthcare/symptom-note/editor/light.png) |  |  |
| **5-4-3-2-1 Calm** | Guides the five-senses grounding exercise one step at a time. | ![5-4-3-2-1 Calm - step five](docs/gallery/png/healthcare/grounding/step-five/light.png) | ![5-4-3-2-1 Calm - step four](docs/gallery/png/healthcare/grounding/step-four/light.png) | ![5-4-3-2-1 Calm - step three](docs/gallery/png/healthcare/grounding/step-three/light.png) | ![5-4-3-2-1 Calm - step two](docs/gallery/png/healthcare/grounding/step-two/light.png) | ![5-4-3-2-1 Calm - step one](docs/gallery/png/healthcare/grounding/step-one/light.png) |

## Games

Fifteen compact games and reaction challenges designed for four-button play on
the e-paper display.

| Application | What it does | Screenshot 1 | Screenshot 2 | Screenshot 3 | Screenshot 4 | Screenshot 5 |
| --- | --- | --- | --- | --- | --- | --- |
| **Morse Letter** | Challenges the player to identify a letter from Morse code. | ![Morse Letter - question](docs/gallery/png/games/morse-letter/question/light.png) | ![Morse Letter - correct](docs/gallery/png/games/morse-letter/correct/light.png) | ![Morse Letter - wrong](docs/gallery/png/games/morse-letter/wrong/light.png) |  |  |
| **Morse Code** | Challenges the player to choose the Morse sequence for a letter. | ![Morse Code - question](docs/gallery/png/games/morse-code/question/light.png) | ![Morse Code - correct](docs/gallery/png/games/morse-code/correct/light.png) | ![Morse Code - wrong](docs/gallery/png/games/morse-code/wrong/light.png) |  |  |
| **Pong** | Plays a compact paddle-and-ball game. | ![Pong - in play](docs/gallery/png/games/pong/in-play/light.png) | ![Pong - paused](docs/gallery/png/games/pong/paused/light.png) | ![Pong - player wins](docs/gallery/png/games/pong/player-wins/light.png) | ![Pong - cpu wins](docs/gallery/png/games/pong/cpu-wins/light.png) |  |
| **Snake** | Grows a snake while avoiding walls and its own trail. | ![Snake - in play](docs/gallery/png/games/snake/in-play/light.png) | ![Snake - paused](docs/gallery/png/games/snake/paused/light.png) | ![Snake - game over](docs/gallery/png/games/snake/game-over/light.png) |  |  |
| **Othello** | Implements the classic disk-flipping strategy game. | ![Othello - human turn](docs/gallery/png/games/othello/human-turn/light.png) | ![Othello - ai thinking](docs/gallery/png/games/othello/ai-thinking/light.png) | ![Othello - human wins](docs/gallery/png/games/othello/human-wins/light.png) | ![Othello - ai wins](docs/gallery/png/games/othello/ai-wins/light.png) | ![Othello - draw](docs/gallery/png/games/othello/draw/light.png) |
| **Rock Paper Scissors** | Plays a scored round against the watch. | ![Rock Paper Scissors - choosing](docs/gallery/png/games/rock-paper-scissors/choosing/light.png) | ![Rock Paper Scissors - win](docs/gallery/png/games/rock-paper-scissors/win/light.png) | ![Rock Paper Scissors - loss](docs/gallery/png/games/rock-paper-scissors/loss/light.png) | ![Rock Paper Scissors - draw](docs/gallery/png/games/rock-paper-scissors/draw/light.png) |  |
| **Reaction Test** | Measures response time after a randomized start signal. | ![Reaction Test - ready](docs/gallery/png/games/reaction-test/ready/light.png) | ![Reaction Test - waiting](docs/gallery/png/games/reaction-test/waiting/light.png) | ![Reaction Test - too soon](docs/gallery/png/games/reaction-test/too-soon/light.png) | ![Reaction Test - result](docs/gallery/png/games/reaction-test/result/light.png) |  |
| **Higher Lower** | Guesses whether the next value will be higher or lower. | ![Higher Lower - guessing](docs/gallery/png/games/higher-lower/guessing/light.png) | ![Higher Lower - correct](docs/gallery/png/games/higher-lower/correct/light.png) | ![Higher Lower - wrong](docs/gallery/png/games/higher-lower/wrong/light.png) | ![Higher Lower - tie](docs/gallery/png/games/higher-lower/tie/light.png) |  |
| **Number Guess** | Narrows down a hidden number using higher and lower hints. | ![Number Guess - initial](docs/gallery/png/games/number-guess/initial/light.png) | ![Number Guess - too low](docs/gallery/png/games/number-guess/too-low/light.png) | ![Number Guess - too high](docs/gallery/png/games/number-guess/too-high/light.png) | ![Number Guess - correct](docs/gallery/png/games/number-guess/correct/light.png) |  |
| **Nim** | Removes objects strategically while playing against the watch. | ![Nim - turn](docs/gallery/png/games/nim/turn/light.png) | ![Nim - player wins](docs/gallery/png/games/nim/player-wins/light.png) | ![Nim - cpu wins](docs/gallery/png/games/nim/cpu-wins/light.png) |  |  |
| **Tic Tac Toe** | Plays a compact three-by-three noughts-and-crosses game. | ![Tic Tac Toe - in play](docs/gallery/png/games/tic-tac-toe/in-play/light.png) | ![Tic Tac Toe - player wins](docs/gallery/png/games/tic-tac-toe/player-wins/light.png) | ![Tic Tac Toe - cpu wins](docs/gallery/png/games/tic-tac-toe/cpu-wins/light.png) | ![Tic Tac Toe - draw](docs/gallery/png/games/tic-tac-toe/draw/light.png) |  |
| **Lights Out** | Toggles a five-by-five grid with the goal of clearing every light. | ![Lights Out - in play](docs/gallery/png/games/lights-out/in-play/light.png) | ![Lights Out - solved](docs/gallery/png/games/lights-out/solved/light.png) |  |  |  |
| **Blackjack** | Plays a simplified hand of blackjack against the dealer. | ![Blackjack - turn](docs/gallery/png/games/blackjack/turn/light.png) | ![Blackjack - hit](docs/gallery/png/games/blackjack/hit/light.png) | ![Blackjack - player wins](docs/gallery/png/games/blackjack/player-wins/light.png) | ![Blackjack - cpu wins](docs/gallery/png/games/blackjack/cpu-wins/light.png) | ![Blackjack - push](docs/gallery/png/games/blackjack/push/light.png) |
| **Quick Math** | Presents short arithmetic questions and tracks correct answers. | ![Quick Math - question](docs/gallery/png/games/quick-math/question/light.png) | ![Quick Math - correct](docs/gallery/png/games/quick-math/correct/light.png) | ![Quick Math - wrong](docs/gallery/png/games/quick-math/wrong/light.png) |  |  |
| **Balance** | Uses the accelerometer to keep an indicator centered. | ![Balance - level](docs/gallery/png/games/balance/level/light.png) | ![Balance - tilted](docs/gallery/png/games/balance/tilted/light.png) | ![Balance - complete](docs/gallery/png/games/balance/complete/light.png) |  |  |

## Clocks

Alternative representations of civil, global, astronomical, and progress-based
time.

| Application | What it does | Screenshot 1 | Screenshot 2 | Screenshot 3 | Screenshot 4 | Screenshot 5 |
| --- | --- | --- | --- | --- | --- | --- |
| **Binary Clock** | Encodes the current hour, minute, and second as binary columns. | ![Binary Clock - default](docs/gallery/png/clocks/binary-clock/default/light.png) |  |  |  |  |
| **Unix Time** | Displays the current Unix epoch timestamp. | ![Unix Time - default](docs/gallery/png/clocks/unix-time/default/light.png) |  |  |  |  |
| **UTC Clock** | Shows the current Coordinated Universal Time. | ![UTC Clock - default](docs/gallery/png/clocks/utc-clock/default/light.png) |  |  |  |  |
| **ISO Week** | Displays the ISO week number and ISO weekday context. | ![ISO Week - default](docs/gallery/png/clocks/iso-week/default/light.png) |  |  |  |  |
| **Day of Year** | Shows the ordinal day and remaining days in the year. | ![Day of Year - default](docs/gallery/png/clocks/day-of-year/default/light.png) |  |  |  |  |
| **Calendar** | Renders a complete month calendar with the current day highlighted. | ![Calendar - default](docs/gallery/png/clocks/month-calendar/default/light.png) |  |  |  |  |
| **World Clocks** | Compares the current time across several world cities. | ![World Clocks - default](docs/gallery/png/clocks/world-clocks/default/light.png) |  |  |  |  |
| **Local + UTC** | Places local time and UTC side by side. | ![Local + UTC - default](docs/gallery/png/clocks/local-utc/default/light.png) |  |  |  |  |
| **Internet Beats** | Converts the day into Swatch Internet Time beats. | ![Internet Beats - default](docs/gallery/png/clocks/internet-beats/default/light.png) |  |  |  |  |
| **Decimal Time** | Represents the current day using decimal hours and subdivisions. | ![Decimal Time - default](docs/gallery/png/clocks/decimal-time/default/light.png) |  |  |  |  |
| **Julian Day** | Displays the astronomical Julian Date. | ![Julian Day - default](docs/gallery/png/clocks/julian-day/default/light.png) |  |  |  |  |
| **Time Progress** | Visualizes progress through the day, month, and year. | ![Time Progress - default](docs/gallery/png/clocks/time-progress/default/light.png) |  |  |  |  |

## Time Tools

Timers, alarms, interval pacing, and rhythm tools.

| Application | What it does | Screenshot 1 | Screenshot 2 | Screenshot 3 | Screenshot 4 | Screenshot 5 |
| --- | --- | --- | --- | --- | --- | --- |
| **Stopwatch** | Measures elapsed time and records lap information. | ![Stopwatch - paused](docs/gallery/png/time-tools/stopwatch/paused/light.png) | ![Stopwatch - running](docs/gallery/png/time-tools/stopwatch/running/light.png) | ![Stopwatch - lap](docs/gallery/png/time-tools/stopwatch/lap/light.png) |  |  |
| **Countdown** | Runs an adjustable countdown timer. | ![Countdown - ready](docs/gallery/png/time-tools/countdown/ready/light.png) | ![Countdown - running](docs/gallery/png/time-tools/countdown/running/light.png) | ![Countdown - finished](docs/gallery/png/time-tools/countdown/finished/light.png) |  |  |
| **Daily Alarm** | Configures a recurring local-time vibration alarm. | ![Daily Alarm - disabled](docs/gallery/png/time-tools/daily-alarm/disabled/light.png) | ![Daily Alarm - enabled](docs/gallery/png/time-tools/daily-alarm/enabled/light.png) |  |  |  |
| **Pomodoro** | Alternates focused work and break countdowns. | ![Pomodoro - ready](docs/gallery/png/time-tools/pomodoro/ready/light.png) | ![Pomodoro - focus](docs/gallery/png/time-tools/pomodoro/focus/light.png) | ![Pomodoro - break](docs/gallery/png/time-tools/pomodoro/break/light.png) | ![Pomodoro - finished](docs/gallery/png/time-tools/pomodoro/finished/light.png) |  |
| **Intervals** | Alternates configurable work and rest periods. | ![Intervals - ready](docs/gallery/png/time-tools/intervals/ready/light.png) | ![Intervals - work](docs/gallery/png/time-tools/intervals/work/light.png) | ![Intervals - rest](docs/gallery/png/time-tools/intervals/rest/light.png) |  |  |
| **Metronome** | Runs a stable 30-240 BPM haptic tempo with remembered accent settings. | ![Metronome - paused](docs/gallery/png/time-tools/metronome/paused/light.png) | ![Metronome - running](docs/gallery/png/time-tools/metronome/running/light.png) |  |  |  |

## Sensors

Battery, motion, activity, health, and runtime diagnostics from Watchy's
on-board hardware.

| Application | What it does | Screenshot 1 | Screenshot 2 | Screenshot 3 | Screenshot 4 | Screenshot 5 |
| --- | --- | --- | --- | --- | --- | --- |
| **Battery Gauge** | Shows calibrated voltage, percentage, and remaining capacity. | ![Battery Gauge - default](docs/gallery/png/sensors/battery-gauge/default/light.png) |  |  |  |  |
| **Power Budget** | Estimates remaining energy and runtime from the battery model. | ![Power Budget - default](docs/gallery/png/sensors/power-budget/default/light.png) |  |  |  |  |
| **Charge Status** | Reports USB presence, charging state, voltage, and charge level. | ![Charge Status - disconnected](docs/gallery/png/sensors/charge-status/disconnected/light.png) | ![Charge Status - charging](docs/gallery/png/sensors/charge-status/charging/light.png) | ![Charge Status - charged](docs/gallery/png/sensors/charge-status/charged/light.png) |  |  |
| **BMA Temperature** | Displays the accelerometer's internal temperature reading. | ![BMA Temperature - default](docs/gallery/png/sensors/bma-temperature/default/light.png) |  |  |  |  |
| **Raw Accel** | Shows raw X, Y, and Z acceleration samples. | ![Raw Accel - default](docs/gallery/png/sensors/raw-accel/default/light.png) |  |  |  |  |
| **G Force** | Converts the acceleration vector into magnitude measured in g. | ![G Force - default](docs/gallery/png/sensors/g-force/default/light.png) |  |  |  |  |
| **Spirit Level** | Uses acceleration to provide a two-axis level indicator. | ![Spirit Level - level](docs/gallery/png/sensors/spirit-level/level/light.png) | ![Spirit Level - tilted](docs/gallery/png/sensors/spirit-level/tilted/light.png) |  |  |  |
| **Orientation** | Classifies the current face-up, face-down, or side orientation. | ![Orientation - face up](docs/gallery/png/sensors/orientation/face-up/light.png) | ![Orientation - face down](docs/gallery/png/sensors/orientation/face-down/light.png) | ![Orientation - right edge](docs/gallery/png/sensors/orientation/right-edge/light.png) |  |  |
| **Motion Score** | Summarizes recent movement intensity as a compact score. | ![Motion Score - still](docs/gallery/png/sensors/motion-score/still/light.png) | ![Motion Score - moving](docs/gallery/png/sensors/motion-score/moving/light.png) | ![Motion Score - active](docs/gallery/png/sensors/motion-score/active/light.png) |  |  |
| **Step Counter** | Displays the BMA423 hardware step count. | ![Step Counter - default](docs/gallery/png/sensors/step-counter/default/light.png) |  |  |  |  |
| **Step Goal** | Shows progress toward the configured daily step target. | ![Step Goal - progress](docs/gallery/png/sensors/step-goal/progress/light.png) | ![Step Goal - complete](docs/gallery/png/sensors/step-goal/complete/light.png) |  |  |  |
| **Walk Distance** | Estimates distance traveled from step count. | ![Walk Distance - default](docs/gallery/png/sensors/walk-distance/default/light.png) |  |  |  |  |
| **Step Calories** | Estimates activity calories from step count. | ![Step Calories - default](docs/gallery/png/sensors/step-calories/default/light.png) |  |  |  |  |
| **Activity State** | Reports the BMA423 activity classifier state. | ![Activity State - stationary](docs/gallery/png/sensors/activity/stationary/light.png) | ![Activity State - walking](docs/gallery/png/sensors/activity/walking/light.png) | ![Activity State - running](docs/gallery/png/sensors/activity/running/light.png) |  |  |
| **Sensor Status** | Summarizes accelerometer enablement, status, errors, and sensor time. | ![Sensor Status - healthy](docs/gallery/png/sensors/sensor-status/healthy/light.png) | ![Sensor Status - error](docs/gallery/png/sensors/sensor-status/error/light.png) |  |  |  |
| **Uptime** | Shows elapsed time since the last cold boot. | ![Uptime - default](docs/gallery/png/sensors/uptime/default/light.png) |  |  |  |  |
| **Shake Counter** | Counts deliberate shakes detected by the accelerometer. | ![Shake Counter - zero](docs/gallery/png/sensors/shake-counter/zero/light.png) | ![Shake Counter - counting](docs/gallery/png/sensors/shake-counter/counting/light.png) |  |  |  |

## Bluetooth

BLE discovery, signal analysis, advertisement inspection, and configurable
Watchy beacon modes.

| Application | What it does | Screenshot 1 | Screenshot 2 | Screenshot 3 | Screenshot 4 | Screenshot 5 |
| --- | --- | --- | --- | --- | --- | --- |
| **BLE Scanner** | Lists nearby Bluetooth Low Energy devices and signal strength. | ![BLE Scanner - scanning](docs/gallery/png/bluetooth/ble-scanner/scanning/light.png) | ![BLE Scanner - result](docs/gallery/png/bluetooth/ble-scanner/result/light.png) | ![BLE Scanner - empty](docs/gallery/png/bluetooth/ble-scanner/empty/light.png) |  |  |
| **Device Count** | Summarizes discovered, named, service-bearing, and manufacturer-bearing devices. | ![Device Count - scanning](docs/gallery/png/bluetooth/device-count/scanning/light.png) | ![Device Count - result](docs/gallery/png/bluetooth/device-count/result/light.png) | ![Device Count - empty](docs/gallery/png/bluetooth/device-count/empty/light.png) |  |  |
| **Strongest Signal** | Highlights the nearest or strongest observed BLE advertiser. | ![Strongest Signal - scanning](docs/gallery/png/bluetooth/strongest-signal/scanning/light.png) | ![Strongest Signal - result](docs/gallery/png/bluetooth/strongest-signal/result/light.png) | ![Strongest Signal - empty](docs/gallery/png/bluetooth/strongest-signal/empty/light.png) |  |  |
| **Named Devices** | Filters scan results to advertisers with readable device names. | ![Named Devices - scanning](docs/gallery/png/bluetooth/named-devices/scanning/light.png) | ![Named Devices - result](docs/gallery/png/bluetooth/named-devices/result/light.png) | ![Named Devices - empty](docs/gallery/png/bluetooth/named-devices/empty/light.png) |  |  |
| **Service UUIDs** | Lists advertised Bluetooth service UUIDs. | ![Service UUIDs - scanning](docs/gallery/png/bluetooth/service-uuids/scanning/light.png) | ![Service UUIDs - result](docs/gallery/png/bluetooth/service-uuids/result/light.png) | ![Service UUIDs - empty](docs/gallery/png/bluetooth/service-uuids/empty/light.png) |  |  |
| **Manufacturer IDs** | Lists manufacturer identifiers found in advertisement data. | ![Manufacturer IDs - scanning](docs/gallery/png/bluetooth/manufacturer-ids/scanning/light.png) | ![Manufacturer IDs - result](docs/gallery/png/bluetooth/manufacturer-ids/result/light.png) | ![Manufacturer IDs - empty](docs/gallery/png/bluetooth/manufacturer-ids/empty/light.png) |  |  |
| **RSSI Bands** | Groups nearby devices into close, near, and far signal bands. | ![RSSI Bands - scanning](docs/gallery/png/bluetooth/rssi-bands/scanning/light.png) | ![RSSI Bands - result](docs/gallery/png/bluetooth/rssi-bands/result/light.png) | ![RSSI Bands - empty](docs/gallery/png/bluetooth/rssi-bands/empty/light.png) |  |  |
| **BLE Addresses** | Lists observed BLE addresses and RSSI values. | ![BLE Addresses - scanning](docs/gallery/png/bluetooth/ble-addresses/scanning/light.png) | ![BLE Addresses - result](docs/gallery/png/bluetooth/ble-addresses/result/light.png) | ![BLE Addresses - empty](docs/gallery/png/bluetooth/ble-addresses/empty/light.png) |  |  |
| **BLE Radar** | Visualizes nearby devices as comparative signal-strength bars. | ![BLE Radar - scanning](docs/gallery/png/bluetooth/ble-radar/scanning/light.png) | ![BLE Radar - result](docs/gallery/png/bluetooth/ble-radar/result/light.png) | ![BLE Radar - empty](docs/gallery/png/bluetooth/ble-radar/empty/light.png) |  |  |
| **TX Power Survey** | Summarizes advertised transmit-power values. | ![TX Power Survey - scanning](docs/gallery/png/bluetooth/tx-power/scanning/light.png) | ![TX Power Survey - result](docs/gallery/png/bluetooth/tx-power/result/light.png) | ![TX Power Survey - empty](docs/gallery/png/bluetooth/tx-power/empty/light.png) |  |  |
| **iBeacon Watch** | Counts nearby iBeacon and Apple manufacturer packets. | ![iBeacon Watch - scanning](docs/gallery/png/bluetooth/ibeacon-watch/scanning/light.png) | ![iBeacon Watch - result](docs/gallery/png/bluetooth/ibeacon-watch/result/light.png) | ![iBeacon Watch - empty](docs/gallery/png/bluetooth/ibeacon-watch/empty/light.png) |  |  |
| **Watchy Beacon** | Advertises a generic Watchy identity over BLE. | ![Watchy Beacon - on air](docs/gallery/png/bluetooth/watchy-beacon/on-air/light.png) | ![Watchy Beacon - paused](docs/gallery/png/bluetooth/watchy-beacon/paused/light.png) |  |  |  |
| **Battery Beacon** | Advertises the standard BLE battery service. | ![Battery Beacon - on air](docs/gallery/png/bluetooth/battery-beacon/on-air/light.png) | ![Battery Beacon - paused](docs/gallery/png/bluetooth/battery-beacon/paused/light.png) |  |  |  |
| **Time Beacon** | Advertises deterministic time data in a service payload. | ![Time Beacon - on air](docs/gallery/png/bluetooth/time-beacon/on-air/light.png) | ![Time Beacon - paused](docs/gallery/png/bluetooth/time-beacon/paused/light.png) |  |  |  |
| **Step Beacon** | Advertises the current step count through a custom service. | ![Step Beacon - on air](docs/gallery/png/bluetooth/step-beacon/on-air/light.png) | ![Step Beacon - paused](docs/gallery/png/bluetooth/step-beacon/paused/light.png) |  |  |  |
| **Name Badge** | Advertises a configured readable device name. | ![Name Badge - on air](docs/gallery/png/bluetooth/name-badge/on-air/light.png) | ![Name Badge - paused](docs/gallery/png/bluetooth/name-badge/paused/light.png) |  |  |  |

## Build And Install

This project targets Watchy v3 / ESP32-S3 and uses PlatformIO:

```powershell
pio run -e esp32-s3-devkitc-1
pio run -e esp32-s3-devkitc-1 -t upload --upload-port COM3
```

Change `COM3` to the port assigned to your Watchy. The upstream hardware setup
guide is available at [watchy.sqfmi.com/docs/getting-started](https://watchy.sqfmi.com/docs/getting-started).

For RTC-retained power proxies such as wake counts, awake time, display work,
radio-on time, sensor wakes, and minimum heap, use the optional diagnostics
build. It writes no telemetry to NVS/Flash and prints `@WATCHY_POWER` records at
115200 baud:

```powershell
pio run -e power-diagnostics -t upload --upload-port COM3
pio device monitor --port COM3 --baud 115200
```

Measured code-path deltas, current binary size, and the remaining physical
measurement matrix are tracked in
[Power Optimization Results](docs/power-optimization-results.md).
The candidate sensor-rate studies are documented separately in
[BCG Sample-Rate Study](docs/bcg-rate-study.md) and
[Step Counter ODR Study](docs/step-odr-study.md). Metronome scheduling evidence
and the pending hardware jitter protocol are in
[Metronome Timing](docs/metronome-timing.md).

## Verification

Run the complete host suite and verify generated gallery metadata before a
pull request:

```powershell
python -m unittest discover -s tools -p "test_*.py"
python tools/sync_gallery_catalog.py --check
pio run -e esp32-s3-devkitc-1
```

The current suite contains 68 tests. The catalog check reports 142
applications and 348 deterministic scenes.

## Application SDK

Applications share standard screens, lists, value editors, semantic input,
feedback, themes, and durable settings. See the
[Watchy Application SDK](docs/watchy-sdk.md)
and the [Standard Counter example](examples/Apps/StandardCounter/StandardCounter.cpp).

## Rebuild The Gallery

The isolated gallery firmware captures every renderer without reading private
settings or activating the panel, radios, sensors, vibration, storage, or
sleep. The receiver rejects incomplete, reordered, duplicate, malformed, or
CRC-invalid frames.

```powershell
python -m pip install -r tools/requirements-gallery.txt
python tools/sync_gallery_catalog.py
pio run -e gallery -t upload --upload-port COM3
python tools/capture_gallery.py --port COM3 --output docs/gallery --replace
python -m unittest tools.test_capture_gallery
pio run -e esp32-s3-devkitc-1 -t upload --upload-port COM3
```

See the [deterministic gallery documentation](docs/deterministic-gallery.md)
and its machine-readable [capture manifest](docs/gallery/manifest.json).

## Project Links

- [Watchy documentation](https://watchy.sqfmi.com/docs/getting-started)
- [Watchy case and accessories](https://shop.sqfmi.com)
- [Community Discord](https://discord.gg/ZXDegGV8E7)
- [Contributing](contributing.md)
- [License](LICENSE)


