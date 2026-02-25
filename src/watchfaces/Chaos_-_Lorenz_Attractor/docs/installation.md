# Installation Guide

**Copyright (c) 2024 Ajey Pai Karkala**

This guide will walk you through setting up the Lorenz Attractor Watch Face on your Watchy ESP32 smartwatch.

## Prerequisites

### Hardware
- Watchy ESP32 smartwatch (any version)
- USB-C cable for programming
- Computer with USB port

### Software
- **Arduino IDE 2.0+** ([Download here](https://www.arduino.cc/en/software))
- **ESP32 Arduino Core 2.0.5+** (installed via Arduino IDE)
- **Watchy Library** (installed via Arduino IDE)

## Step-by-Step Installation

### 1. Install Arduino IDE

1. Download Arduino IDE 2.0+ from the official website
2. Install following the platform-specific instructions
3. Launch Arduino IDE

### 2. Install ESP32 Arduino Core

1. Open Arduino IDE
2. Go to **File → Preferences**
3. In "Additional Boards Manager URLs", add:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
4. Go to **Tools → Board → Boards Manager**
5. Search for "ESP32" and install "esp32 by Espressif Systems"
6. Ensure version 2.0.5 or higher is installed

### 3. Install Watchy Library

1. In Arduino IDE, go to **Tools → Manage Libraries**
2. Search for "Watchy"
3. Install "Watchy" by SQFMI
4. Wait for installation to complete

### 4. Download the Lorenz Attractor Code

1. Clone or download this repository:
   ```bash
   git clone https://github.com/yourusername/lorenz-attractor-watchy.git
   ```
2. Or download as ZIP and extract

### 5. Configure Arduino IDE

1. Open Arduino IDE
2. Go to **Tools → Board → ESP32 Arduino → Watchy**
3. Select the correct USB port for your Watchy
4. Ensure these settings are selected:
   - **Board**: "Watchy"
   - **Upload Speed**: "921600"
   - **CPU Frequency**: "240MHz (WiFi/BT)"
   - **Flash Frequency**: "80MHz"
   - **Flash Mode**: "QIO"
   - **Flash Size**: "4MB (32Mb)"
   - **Partition Scheme**: "Default 4MB with spiffs"
   - **Core Debug Level**: "None"
   - **PSRAM**: "Disabled"

### 6. Upload the Code

1. Open `chaos/chaos.ino` in Arduino IDE
2. Connect your Watchy via USB-C cable
3. Press the reset button on your Watchy
4. Click the **Upload** button (arrow icon) in Arduino IDE
5. Wait for compilation and upload to complete

### 7. First Boot

1. Disconnect the USB cable
2. Press the reset button on your Watchy
3. The Lorenz attractor should start appearing immediately
4. The pattern will begin evolving every 0.5 seconds
5. Moon phase and sunrise/sunset will display based on configured location

## Troubleshooting

### Upload Issues

**"Port not found"**:
- Check USB cable connection
- Try a different USB port
- Install USB drivers if needed

**"Failed to connect"**:
- Press and hold the reset button while uploading
- Try different upload speed (115200)
- Check USB cable quality

**Compilation errors**:
- Ensure ESP32 Arduino Core 2.0.5+ is installed
- Check Watchy library version
- Verify board selection

### Runtime Issues

**Pattern not appearing**:
- Check if code uploaded successfully
- Verify Watchy is powered on
- Try resetting the device

**Pattern not evolving**:
- Wait a few minutes for RTC memory to initialize
- Check battery level
- Verify wake-up timer is working

**Display issues**:
- Check screen for physical damage
- Try resetting the device
- Verify code compilation

## Verification

After successful installation, you should see:

1. **Time display** in 24-hour format
2. **Date** showing current day and date
3. **Moon phase indicator** showing current lunar phase
4. **Battery indicator** showing percentage (0-100%)
5. **Step counter** showing your steps
6. **Sunrise/sunset times** based on your location
7. **Lorenz attractor pattern** that evolves every 0.5 seconds

## Next Steps

- Read the [Configuration Guide](configuration.md) to customize settings
- Check the [Technical Documentation](technical.md) for advanced details
- Explore [Example Configurations](../examples/) for different setups

## Support

If you encounter issues:

1. Check the [Troubleshooting](#troubleshooting) section above
2. Review the [Technical Documentation](technical.md)
3. Open an issue on GitHub with detailed information
4. Join the Watchy community forums

---

**Happy watching!** 🕰️✨
