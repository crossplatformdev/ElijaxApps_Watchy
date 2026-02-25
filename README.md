# Glitchy — Extended Watchy Firmware

**Version 1.0** — A feature-rich firmware for [SQFMI Watchy](https://watchy.sqfmi.com/) smartwatches

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![Platform: ESP32-S3](https://img.shields.io/badge/ESP32-S3-green)](https://www.espressif.com/en/products/socs/esp32-s3)
[![PlatformIO](https://img.shields.io/badge/Built_with-PlatformIO-orange)](https://platformio.org/)

---

## Overview

**Glitchy** is a comprehensive firmware for Watchy smartwatches built on ESP32-S3. It integrates **76+ watchfaces** from the community, a unified **UiSDK framework** for consistent rendering, and a full suite of **network-enabled apps** for productivity and diagnostics.

### Key Features

- 🎨 **76+ Watchfaces** — Analog, digital, retro, pixel art, and game-themed designs
- 🌓 **Dark/Light Theme** — Global theme with automatic bitmap inversion
- 🛜 **Network Apps** — WiFi connectivity, NTP sync, weather, ping, traceroute, port scanner, DNS lookup, WHOIS, DuckDuckGo search, news reader, and more
- 📱 **BLE OTA Updates** — Over-the-air firmware updates via Bluetooth
- ⏰ **Built-in Apps** — Alarm, chronometer, timer, accelerometer, moon phase, sunrise/sunset, Morse code game
- 📖 **Text Browser** — View and scroll through text documents
- 🔧 **WiFi Manager** — Easy setup and connection management

---

## Screenshots

| Watchface Examples | Apps |
|-------------------|------|
| ![7_SEG](docs/images/7seg.png) ![Analog](docs/images/analog.png) ![Pokemon](docs/images/pokemon.png) | ![Menu](docs/images/menu.png) ![Weather](docs/images/weather.png) |

*76+ watchfaces with themes from retro LCD displays to modern digital art*

---

## Watchfaces Gallery

<details>
<summary>Click to expand full watchface list (76 total)</summary>

### A-B
- **7_SEG** — Classic 7-segment LCD display
- **7_SEG_LIGHT** — Light variant of 7-segment
- **Analog** — Traditional analog clock with hands
- **Bad_For_Eye** — High-contrast glitch aesthetic
- **Bahn** — Railway station inspired design
- **Basic** — Minimalist time display
- **BCD** — Binary-coded decimal representation
- **beastie** — BSD daemon mascot
- **Big_Time** — Large, bold digits
- **Binary** — Binary time representation
- **BinaryBlocks** — Block-based binary clock
- **BotWatchy** — *The Legend of Zelda: Breath of the Wild* HUD
- **Brainwork** — Collection of artistic designs

### C-D
- **BTTF** — *Back to the Future* time circuit
- **Calculator_Watchy** — Functional calculator interface
- **Calculateur** — French calculator design
- **Calendar_WatchFace** — Monthly calendar view
- **Captn_Wednesday** — Nautical themed display
- **Castle_of_Watchy** — *Castle of Heart* game inspired
- **Chaos_-_Lorenz_Attractor** — Mathematical visualization
- **Chronometer** — Professional chronograph layout
- **CityWeather** — Location-aware weather display
- **Dali** — Surrealist melting clock
- **dkTime** — Danish design aesthetic
- **Digdug_Watch** — *Dig Dug* arcade game tribute
- **DOS** — MS-DOS terminal interface

### E-M
- **erika_Type** — Typewriter inspired font
- **Exactly-Words** — Written time format ("ten thirty")
- **Hobbit_Time** — *The Hobbit* themed design
- **Jarvis** — *Iron Man* AI interface
- **Kave_Watchy** — *Jet Kave Adventure* HUD
- **Keen** — *Commander Keen* game aesthetic
- **Kitty** — Cat photo with time overlay
- **Last_Laugh** — *Batman* Joker theme
- **LCARS** — *Star Trek* computer interface
- **Line** — Minimalist line-based time
- **MacPaint** — Classic Mac bitmap art
- **Mario** — *Super Mario Bros.* pixel art
- **Marquee** — Isometric billboard scene
- **Maze** — Random maze with time
- **MetaBall** — Animated blob simulation
- **Mickey** — *Mickey Mouse* cartoon style
- **Multi_face_Watchy** — Multiple face switching

### O-S
- **Orbital** — Planetary orbit visualization
- **Pip-Boy** — *Fallout* game interface
- **Poe** — Gothic literature tribute
- **Pokemon** — *Pokémon* Game Boy style
- **pxl999** — Pixelated weather display
- **QArtCode** — QR code with custom image
- **QLock** — Word clock (quarter, half, o'clock)
- **QR_Watchface** — Pure QR code time
- **Re-Dub** — Reggae/dub aesthetic
- **Revolution** — French Revolutionary calendar
- **S2Analog** — Circular analog layout
- **Shadow_Clock** — 3D shadow projection
- **Shijian** — Chinese characters time
- **Skully** — Skull pixel art
- **Skykid_Watch** — *Sky Kid* arcade game
- **Slacker** — Linux terminal theme
- **SmartWatchy** — Feature-packed smart display
- **Spiral_Watchy** — Spiral time visualization
- **Squarbital** — Square orbital animation
- **Squaro** — Blocky digit design
- **Star_Wars_Aurebesh** — *Star Wars* alien script
- **StarryHorizon** — Night sky with stars
- **Stationary_Text** — Fixed text layout
- **Steps** — Step counter focus
- **SW_Watchy** — *Star Wars* Galactic Basic
- **Sundial** — Sun position timekeeping

### T-X
- **Tetris** — *Tetris* block arrangement
- **The_Blob** — Morphing blob animation
- **Triangle** — Geometric triangle pattern
- **TypoStyle** — Typography focused design
- **Watchy_Akira** — *Akira* anime aesthetic
- **Watchy_PowerShell** — Windows PowerShell terminal
- **WatchySevenSegment** — Advanced 7-segment with stopwatch
- **X_marks_the_spot** — Treasure map style

</details>

---

## Built-in Apps

### Core Apps
- **About** — Firmware version and device info
- **Accelerometer** — Real-time motion sensor data
- **Alarm** — Set wake-up alarms with vibration
- **Buzz** — Manual vibration motor test
- **Chronometer** — Stopwatch with lap timer
- **Timer** — Countdown timer
- **Time** — Current time and date display

### Network Apps
- **Connect WiFi** — Quick WiFi connection
- **Setup WiFi** — WiFi network configuration
- **Sync NTP** — Manual time synchronization
- **DNS LookUp** — Resolve domain names to IPs
- **DuckDuckGo Search** — Privacy-focused web search
- **News Reader** — RSS feed reader
- **Ping** — ICMP echo request diagnostics
- **Port Scanner** — TCP port availability checker
- **Postman** — HTTP API testing tool
- **Traceroute** — Network hop visualization
- **Whois** — Domain registration lookup

### Astronomy Apps
- **Moon Phase** — Current lunar phase calculation
- **MoonRise** — Moon rise/set times
- **SunRise** — Sunrise/sunset calculator

### Miscellaneous
- **Morse Game** — Learn Morse code interactively
- **Radio** — (placeholder for future extension)
- **Text Browser** — Read long text documents with scrolling
- **Theme** — Toggle between dark/light modes
- **Firmware Update** — BLE OTA update interface

---

## UiSDK Framework

All watchfaces and apps use the **UiSDK** (Unified Interface SDK) for consistent rendering. The SDK provides:

- **Display wrappers** — All drawing primitives (pixels, lines, shapes, text, bitmaps)
- **Theme system** — Automatic dark/light mode with polarity-aware inversion
- **Spec-based UI** — Declarative layouts using structured specs (`UITextSpec`, `UIImageSpec`, `UIMenuSpec`, etc.)
- **Font registry** — Centralized font management
- **Input helpers** — Debounced button reading

### Quick Example

```cpp
#include "sdk/UiSDK.h"

void draw(Watchy &watchy) {
    UiSDK::setPolarity(WatchfacePolarity::BlackOnWhite);
    auto &display = watchy.display;
    
    uint16_t bg = UiSDK::getWatchfaceBg(BASE_POLARITY);
    uint16_t fg = UiSDK::getWatchfaceFg(BASE_POLARITY);
    
    UiSDK::setFullWindow(display);
    UiSDK::fillScreen(display, bg);
    UiSDK::setTextColor(display, fg);
    UiSDK::setFont(display, &FreeMonoBold18pt7b);
    UiSDK::setCursor(display, 30, 100);
    UiSDK::print(display, "Hello Watchy!");
    UiSDK::displayUpdate(display);
}
```

👉 **See [SDK_REFERENCE.md](SDK_REFERENCE.md) for complete API documentation**

---

## Getting Started

### Hardware Requirements

- **SQFMI Watchy** (ESP32-S3 variant recommended)
- **USB-C cable** (for programming and power)

### Software Requirements

- **[PlatformIO](https://platformio.org/)** (CLI or IDE)
- **Python 3.x** (for build tools)
- **Git** (for cloning repository)

### Building and Flashing

1. **Clone the repository:**
   ```bash
   git clone https://github.com/yourusername/glitchy-watchy
   cd glitchy-watchy
   ```

2. **Install dependencies:**
   PlatformIO will auto-install all dependencies from `platformio.ini`

3. **Build the firmware:**
   ```bash
   platformio run --environment esp32-s3-devkitc-1
   ```

4. **Flash to device:**
   ```bash
   platformio run --target upload --environment esp32-s3-devkitc-1
   ```

5. **Monitor serial output (optional):**
   ```bash
   platformio device monitor --environment esp32-s3-devkitc-1
   ```

### First-Time Setup

1. **Set WiFi credentials:**
   - Navigate to: **Menu → Setup WiFi**
   - Select your network and enter password
   - WiFi credentials persist across reboots

2. **Sync time:**
   - Navigate to: **Menu → Sync NTP**
   - Connects to NTP server and updates RTC

3. **Select watchface:**
   - Hold **MENU** button (top-left) for 2 seconds
   - Navigate with **UP/DOWN** buttons
   - Press **MENU** to confirm selection

---

## Project Structure

```
.
├── src/
│   ├── apps/           # Built-in applications
│   ├── os/             # OS core (menu system, watchface dispatcher)
│   ├── sdk/            # UiSDK framework (UiSDK.h, UiTemplates.h, Fonts.h)
│   ├── settings/       # Persistent settings storage
│   ├── watchy/         # Watchy hardware abstraction (Watchy.cpp, RTC, display)
│   └── watchfaces/     # 76 watchface implementations
├── examples/
│   └── HelloWorldApp/  # Minimal example app
├── tools/              # Python scripts for watchface import/analysis
├── include/            # Platform-specific headers
├── lib/                # External libraries (if any)
├── test/               # Unit tests (placeholder)
├── platformio.ini      # PlatformIO build configuration
├── partitions_8mb_huge_app.csv  # ESP32 flash partition table
├── sdkconfig.esp32-s3-devkitc-1  # ESP-IDF SDK config
├── SDK_REFERENCE.md    # Complete UiSDK API documentation
├── FIRMWARE_MANUAL.md  # User manual (buttons, menus, apps)
├── THEME_SYSTEM.md     # Theme/polarity deep dive
├── LICENSE             # GPLv3 license
├── AUTHORS.txt         # Credits and contributors
└── README.md           # This file
```

---

## Documentation

- **[SDK_REFERENCE.md](SDK_REFERENCE.md)** — Complete UiSDK API reference
- **[FIRMWARE_MANUAL.md](FIRMWARE_MANUAL.md)** — User guide (buttons, menus, features)
- **[THEME_SYSTEM.md](THEME_SYSTEM.md)** — How theme inversion works
- **[examples/HelloWorldApp/](examples/HelloWorldApp/)** — Tutorial app
- **[tools/watchface_polarities_report.md](tools/watchface_polarities_report.md)** — Watchface polarity analysis

---

## Contributing

Contributions are welcome! Please follow these guidelines:

1. **Fork** the repository
2. **Create a feature branch**: `git checkout -b feature/your-feature`
3. **Use UiSDK functions** for all display operations (see [SDK_REFERENCE.md](SDK_REFERENCE.md))
4. **Set watchface polarity** correctly (`UiSDK::setPolarity()`)
5. **Test with both themes** (dark and light mode)
6. **Submit a pull request** with clear description

### Adding a New Watchface

1. Create folder `src/watchfaces/YourWatchface/`
2. Implement `YourWatchface.cpp` with `draw(Watchy &watchy)` function
3. Add entry to `src/sdk/WatchfaceRegistry.cpp`
4. Add menu item to watchface selector
5. Set correct `WatchfacePolarity` in `UiSDK::polarityForFaceId()`
6. Build and test with both themes

👉 See [examples/HelloWorldApp/](examples/HelloWorldApp/) for reference implementation

---

## Known Issues

- **QArtCode watchface**: May hang on slow network (timeout added, but slow servers can still cause delays)
- **Memory constraints**: With 76 watchfaces compiled, ROM usage is ~68% (4.8 MB / 7.0 MB available)
- **Deep sleep accuracy**: RTC drift mitigation implemented (microsecond alignment), but crystal tolerance affects long-term accuracy

Workarounds and fixes are documented in [FIRMWARE_MANUAL.md](FIRMWARE_MANUAL.md#troubleshooting)

---

## License

This project is licensed under the **GNU General Public License v3.0** (GPLv3).

You are free to:
- ✅ Use this firmware on your Watchy device
- ✅ Modify and redistribute under the same license
- ✅ Use for commercial purposes (hardware sales, etc.)

You must:
- ⚠️ Disclose source code when distributing
- ⚠️ Include a copy of GPLv3 license
- ⚠️ State changes made to the original code

See [LICENSE](LICENSE) file for full terms.

---

## Credits

### Original Watchy Firmware
- **SQFMI (squarofumi)** — Original Watchy hardware and firmware  
  https://github.com/sqfmi/Watchy

### Primary Developer
- **Elías A. Angulo Klein** ([crossplatformdev](https://github.com/crossplatformdev))  
  Firmware integration, UiSDK framework, theme system, app suite, build system

### Watchface Authors
This firmware integrates 76+ community watchfaces. See [AUTHORS.txt](AUTHORS.txt) for complete credits.

Notable contributors:
- **Bill Eichner** — Multi_face_Watchy, Slacker, Calendar_WatchFace
- **peerdavid** — BTTF, LCARS, BCD, Steps, Jarvis
- **My-Key** — Castle_of_Watchy, Kave_Watchy, Skykid_Watch, Digdug_Watch, Dali
- **rontalman** — Marquee (pixel art)
- **75thTrombone** — BotWatchy resources
- *...and many others* (see [AUTHORS.txt](AUTHORS.txt))

### Libraries and Dependencies
- **Adafruit GFX Library** — Graphics primitives
- **GxEPD2** — E-paper display driver
- **ESP32 Arduino Core** — Espressif ESP32 support
- **NTPClient** — Network time synchronization
- **WiFiManager** — WiFi configuration portal
- **QRCode** — QR code generation
- **ArduinoJson** — JSON parsing
- **Sunset / MoonPhase / MoonRise / SunRise** — Astronomy calculations

Full dependency list in [platformio.ini](platformio.ini)

---

## Community

- **Watchy Official Discord**: https://discord.gg/watchy
- **SQFMI Forum**: https://forum.sqfmi.com/
- **Reddit**: [r/Watchy](https://reddit.com/r/Watchy)

**Report issues**: https://github.com/yourusername/glitchy-watchy/issues  
**Discussions**: https://github.com/yourusername/glitchy-watchy/discussions

---

## Changelog

### Version 1.0 (2025-01-XX)
- ✨ Initial release
- ✨ 76 integrated watchfaces
- ✨ UiSDK framework with theme system
- ✨ 20+ built-in apps (network, astronomy, games)
- ✨ BLE OTA updates
- ✨ Comprehensive documentation suite
- 🐛 Fixed clock drift issue (microsecond deep-sleep alignment)
- 🐛 Fixed QArtCode hang (HTTP timeout + fallback)
- 🔧 Mass SDK migration (all watchfaces use `UiSDK::*` functions)

---

**Built with ❤️ for the Watchy community**

If you find this firmware useful, please ⭐ star the repository and share with other Watchy users!
