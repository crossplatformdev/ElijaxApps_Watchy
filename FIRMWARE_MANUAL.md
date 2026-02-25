# Watchy Firmware Manual (ElijaxApps)

Version: 1.0  
Target hardware: Watchy on ESP32-S3 (`esp32-s3-devkitc-1`)

---

## 1) What this firmware is

ElijaxApps is a Watchy firmware build that combines:

- A unified rendering SDK (`UiSDK`) for apps and watchfaces
- A grouped menu system with button-driven navigation
- 76 watchfaces selectable from device UI
- Built-in apps for timekeeping, networking, astronomy, utilities, and diagnostics

The main firmware entrypoint is in [src/main.cpp](src/main.cpp).

---

## 2) Hardware buttons and behavior

Watchy uses four physical buttons:

- **MENU** (top-left)
- **BACK** (bottom-left)
- **UP** (top-right)
- **DOWN** (bottom-right)

Button handling is implemented in [src/watchy/Watchy.cpp](src/watchy/Watchy.cpp) and callback contracts in [src/watchy/Watchy.h](src/watchy/Watchy.h).

### Global behavior

- From watchface state:
  - `MENU` enters main menu
- Inside menu:
  - `UP`/`DOWN` move selection
  - `MENU` enters submenu or launches selected app
  - `BACK` goes to previous level or exits to watchface
- Inside an app:
  - behavior depends on app; most screens map controls via `UiTemplates` and `UiSDK`

---

## 3) System states

Core GUI states are managed by `guiState` (RTC-persistent), with transitions in [src/watchy/Watchy.cpp](src/watchy/Watchy.cpp).

Typical states:

- **WATCHFACE_STATE**: normal watchface display
- **MAIN_MENU_STATE**: category/submenu navigation
- **APP_STATE**: active app screen
- **FW_UPDATE_STATE**: firmware update flow

The firmware restores behavior after wake from deep sleep according to wake reason:

- Timer wake: update watchface / menu timeout behavior
- EXT1 wake: button press handling
- USB wake (ESP32-S3): refresh display and return to watchface view

---

## 4) Sleep, wake, and timekeeping

Deep sleep is configured in [src/watchy/Watchy.cpp](src/watchy/Watchy.cpp) (`Watchy::deepSleep`).

### Key design points

- CPU reduced to 80 MHz before sleep for power savings
- Display enters hibernate mode before sleep
- Wake sources:
  - button wake (EXT1)
  - timer wake
  - USB state wake (ESP32-S3 path)
- Timer wake interval aligns to the **next minute boundary** using `gettimeofday()` microseconds

### Drift strategy

- Minute updates are aligned using microsecond timing to reduce long-term display drift perception.
- No mandatory periodic auto-NTP in the core watch loop.
- Time can be updated manually with **Sync with NTP** app flow.

---

## 5) Menu structure

Menu definitions are in [src/apps/MenuApp.cpp](src/apps/MenuApp.cpp).

Top-level categories:

1. **Time & Alarms**
2. **System Settings**
3. **Sensors**
4. **Network Tools**
5. **Apps**
6. **Astronomy**
7. **Preferences**

### 5.1 Time & Alarms

- Alarm
- Timer
- Chronometer

### 5.2 System Settings

- Set up WiFi
- Set up Time
- Sync with NTP

### 5.3 Sensors

- Vibrate Motor
- Show Accelerometer

### 5.4 Network Tools

- Ping
- Traceroute
- DNS Look-up
- Whois
- Port scan
- HTTP client

### 5.5 Apps

- Web Search
- Text browser
- News
- Radio
- Morse Game

### 5.6 Astronomy

- Sun rise/set
- Moon rise/set
- Moon phase

### 5.7 Preferences

- Watchfaces
- Theme
- Update FW
- About Watchy

---

## 6) Watchface selector

Watchface selector implementation is in [src/os/WatchFaceSelectorApp.cpp](src/os/WatchFaceSelectorApp.cpp).

Controls in selector:

- `BACK`: cancel and return
- `UP`/`DOWN`: navigate watchface list
- `SET` (`MENU`): save selected watchface (`currentWatchfaceId`)

Watchface registry is defined in [src/sdk/WatchfaceRegistry.cpp](src/sdk/WatchfaceRegistry.cpp), currently with **76 entries**.

---

## 7) Theme system (dark/light)

Theme behavior is centered around `gDarkMode` and polarity-aware rendering.

Relevant files:

- [THEME_SYSTEM.md](THEME_SYSTEM.md)
- [src/sdk/UiSDK.h](src/sdk/UiSDK.h)
- [src/sdk/UiSDK.cpp](src/sdk/UiSDK.cpp)
- [src/watchy/ThemeableDisplay.h](src/watchy/ThemeableDisplay.h)

### How it works

- Global theme toggles dark/light output (`gDarkMode`)
- Each watchface has authored polarity (white-on-black or black-on-white)
- SDK computes effective foreground/background and inversion behavior per face

---

## 8) Apps reference

All app entrypoints are methods on `Watchy` declared in [src/watchy/Watchy.h](src/watchy/Watchy.h), with implementations under [src/apps](src/apps).

### 8.1 Time & utility apps

- **AboutApp**: build/device information
- **AlarmApp**: alarm configuration
- **TimerApp**: countdown timer
- **ChronometerApp**: stopwatch behavior
- **TimeApp**: time/date controls
- **ThemeApp**: dark/light theme toggle
- **FirmwareUpdateApp**: OTA update flow

### 8.2 Sensor apps

- **AccelerometerApp**: accelerometer readout
- **BuzzApp**: vibration motor testing

### 8.3 Connectivity setup

- **SetupWifiApp**: first-time/config portal flow
- **ConnectWifiApp**: connect to configured network
- **SyncNTPApp**: manual network time synchronization

### 8.4 Network diagnostic apps

- **PingApp**
- **TracerouteApp**
- **DNSLookUpApp**
- **WhoisApp**
- **PortScannerApp**
- **PostmanApp** (HTTP client)

### 8.5 Content/online apps

- **DuckDuckGoSearchApp**
- **TextBrowserApp**
- **NewsReaderApp**
- **RadioApp**
- **MorseGame**

### 8.6 Astronomy apps

- **SunRiseApp**
- **MoonRiseApp**
- **MoonPhaseApp**

---

## 9) Networking and data behavior

### WiFi

- WiFi setup and connection are app-driven (manual initiation)
- Credentials may be cached/persisted in RTC/global settings paths

### NTP

- NTP sync is available via explicit app action (`Sync with NTP`)
- Firmware does not require periodic background auto-sync to keep running

### Weather / online APIs

Weather and online utilities use HTTP requests with timeout handling and fallback behavior where implemented.

---

## 10) Battery, sensors, and board support

### Battery voltage

- Read via ADC path (`getBatteryVoltage`) with board-specific conversion factors

### Motion sensor

- BMA423 setup occurs in `_bmaConfig()` in [src/watchy/Watchy.cpp](src/watchy/Watchy.cpp)
- Features enabled include step counting, tilt, and wakeup interrupts

### Board revisions

- Board revision detection logic is in `getBoardRevision()`

---

## 11) Build and flash

Build configuration is in [platformio.ini](platformio.ini).

### Build

`platformio run --environment esp32-s3-devkitc-1`

### Upload

`platformio run --target upload --environment esp32-s3-devkitc-1`

### Serial monitor

`platformio device monitor --environment esp32-s3-devkitc-1`

---

## 12) Recovery and troubleshooting

### Device wakes but menu feels stuck

- Ensure no button is mechanically stuck
- Reboot device
- Verify control capture transitions in apps based on `UiTemplates`

### Time appears off

- Run **Sync with NTP** once with WiFi available
- Confirm timezone/GMT settings in settings
- Recheck after several sleep/wake cycles

### Watchface renders incorrectly on theme switch

- Confirm watchface polarity mapping in [src/sdk/UiSDK.cpp](src/sdk/UiSDK.cpp)
- Confirm face uses `UiSDK` wrappers and theme colors

### Build errors after adding a watchface/app

- Verify includes and namespace use
- Ensure file is referenced by registry/dispatch if required
- Rebuild cleanly from PlatformIO

---

## 13) Developer notes

### Required rendering path

- Use `UiSDK::*` wrappers for drawing operations
- Avoid direct raw display calls in migrated watchfaces

### UI specs

- Prefer `UIAppSpec` + element specs for consistent behavior
- Use `UiTemplates` helpers for control rows/menu pickers/history pages

### Docs references

- SDK API: [SDK_REFERENCE.md](SDK_REFERENCE.md)
- Theme behavior: [THEME_SYSTEM.md](THEME_SYSTEM.md)
- Example app: [examples/HelloWorldApp/README.md](examples/HelloWorldApp/README.md)

---

## 14) Legal and credits

- License: GNU GPLv3 in [LICENSE](LICENSE)
- Credits and upstream attributions: [AUTHORS.md](AUTHORS.md)

This firmware aggregates works from multiple authors and repositories.
Please keep attributions intact when redistributing.
