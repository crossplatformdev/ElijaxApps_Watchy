# Chaos - Lorenz Attractor Watch Face for Watchy

A mesmerizing watch face for the Watchy ESP32 smartwatch featuring a real-time evolving Lorenz attractor pattern. Watch the beautiful chaos unfold on your wrist with a continuously updating 3D trajectory that rotates and evolves. Includes moon phase display and sunrise/sunset times based on your location.

![Chaos Watchy Demo](examples/gifs/chaos_watchy_demo.gif)

*Demo animation showing the Lorenz attractor pattern on the Watchy display*

## Features

- **Real-time Lorenz Attractor**: Live 3D chaotic system simulation using Runge-Kutta 4th order integration
- **Persistent State**: RTC memory storage ensures continuous evolution across deep sleep cycles
- **Fast Updates**: 0.5-second refresh rate with 5 new trajectory points each update
- **3D Rotation**: Dynamic rotating view of the attractor pattern
- **Moon Phase Display**: Accurate lunar cycle visualization with real-time phase calculation
- **Sunrise/Sunset Times**: Location-based solar calculations showing daily sun times
- **Improved Battery Indicator**: Percentage-based battery display instead of binary full/empty
- **Large Display**: 190x190 pixel drawing area (95% of screen)
- **Standard Watch Features**: Time, date, and step counter
- **Power Efficient**: Optimized for Watchy's deep sleep architecture with custom sleep/wake management

## Technical Details

### Lorenz System Parameters
- **σ (Sigma)**: 10.0 - Prandtl number
- **ρ (Rho)**: 28.0 - Rayleigh number  
- **β (Beta)**: 8/3 - Geometric factor
- **Time Step**: 0.05 - Integration step size
- **Max Points**: 500 - Circular buffer size

### Performance
- **Integration Steps**: 5 per update (0.25 time units)
- **Wake-up Frequency**: Every 0.5 seconds
- **Evolution Rate**: 600 points per minute
- **Memory Usage**: RTC persistent state variables

### Additional Features
- **Moon Phase**: Calculated using 29.53-day lunar cycle with timezone support
- **Sunrise/Sunset**: Solar calculations based on latitude/longitude and timezone
- **Battery Display**: Percentage-based (3.0V - 4.2V range)

## Project Structure

```
chaos/
├── chaos/
│   ├── chaos.ino             # Main watch face implementation
│   └── settings.h            # Configuration parameters
├── docs/
│   ├── installation.md       # Setup and installation guide
│   ├── configuration.md      # Customization options
│   ├── technical.md         # Technical implementation details
│   └── images/              # Documentation images
├── examples/
│   ├── custom_configs/      # Example configuration files
│   └── gifs/                # Demo GIF animations
├── tools/
│   └── chaos_demo.py        # GIF generator script
├── LICENSE                  # MIT License
└── README.md               # This file
```

## Quick Start

### Prerequisites
- Arduino IDE 2.0+
- ESP32 Arduino Core 2.0.5+
- Watchy library
- Watchy ESP32 smartwatch

### Installation

1. **Clone the repository**:
   ```bash
   git clone https://github.com/yourusername/lorenz-attractor-watchy.git
   cd lorenz-attractor-watchy
   ```

2. **Install Watchy library**:
   - Open Arduino IDE
   - Go to Tools → Manage Libraries
   - Search for "Watchy" and install

3. **Configure Arduino IDE**:
   - Select Board: "Watchy" (ESP32)
   - Select Port: Your Watchy's USB port
   - Ensure ESP32 Arduino Core 2.0.5+ is installed

4. **Upload the code**:
   - Open `chaos/chaos.ino` in Arduino IDE
   - Click Upload
   - Wait for upload to complete

### First Run
- The watch will automatically start showing the Lorenz attractor
- The pattern will begin evolving immediately
- Wake-up every 0.5 seconds for continuous evolution
- Moon phase and sunrise/sunset will display based on configured location

## Demo GIF Generator

The `tools/chaos_demo.py` script generates a preview GIF showing how the Lorenz attractor appears on the Watchy display:

```bash
cd tools
python3 chaos_demo.py
```

This creates `chaos_watchy_demo.gif` in the `examples/gifs/` directory, simulating the actual watch face behavior with:
- 3D rotating Lorenz attractor pattern
- Moon phase indicator
- Sunrise/sunset times
- Watch elements (time, date, battery percentage, steps)
- E-ink display simulation
- 60-frame animation at 10 FPS

## Configuration

Edit `chaos/settings.h` to customize behavior:

```cpp
// Wake up settings
#define WAKE_UP_INTERVAL_MINUTES (0.5/60.0)  // 0.5 seconds

// Lorenz attractor settings  
#define LORENZ_POINTS_PER_UPDATE 5           // Points per refresh
#define LORENZ_MAX_POINTS 500                // Trajectory buffer size
#define LORENZ_ROTATION_SPEED 0.0523598776  // Rotation speed (3 degrees per update)

// Display refresh settings
#define FULL_REFRESH_INTERVAL 60             // Full refresh every N updates (60 = 30 seconds)

// Location and timezone settings
#define TIMEZONE_OFFSET_HOURS 1              // Your timezone offset from UTC
#define LATITUDE 52.3676                     // Your latitude
#define LONGITUDE 4.9041                     // Your longitude
```

## Customization

### Adjusting Evolution Speed
- **Faster evolution**: Increase `LORENZ_POINTS_PER_UPDATE`
- **Slower evolution**: Decrease `LORENZ_POINTS_PER_UPDATE`
- **More detail**: Increase `LORENZ_MAX_POINTS`

### Changing Wake-up Frequency
- **Faster updates**: Decrease `WAKE_UP_INTERVAL_MINUTES`
- **Battery saving**: Increase `WAKE_UP_INTERVAL_MINUTES`

### Display Refresh Strategy
- **More frequent full refreshes**: Decrease `FULL_REFRESH_INTERVAL` (reduces ghosting but drains battery)
- **Less frequent full refreshes**: Increase `FULL_REFRESH_INTERVAL` (saves battery but may show ghosting)
- **Note**: The watch uses partial refreshes between full refreshes to preserve battery and reduce screen wear

### Visual Adjustments
- **Larger pattern**: Modify window bounds in `drawTrajectory()`
- **Different rotation**: Adjust `LORENZ_ROTATION_SPEED`

## Troubleshooting

### Common Issues

**Pattern not evolving**:
- Ensure RTC memory is working (variables prefixed with `RTC_DATA_ATTR`)
- Check that wake-up timer is functioning
- Verify integration parameters

**Compilation errors**:
- Ensure ESP32 Arduino Core 2.0.5+ is installed
- Check Watchy library version compatibility
- Verify board selection is "Watchy"

**Display issues**:
- Check screen bounds in `drawTrajectory()`
- Verify scaling parameters
- Ensure proper centering

**Screen ghosting/burn-in**:
- Reduce `FULL_REFRESH_INTERVAL` for more frequent full refreshes (e.g., 30 = 15 seconds)
- E-ink displays naturally have some ghosting with partial refreshes
- Full refreshes every 30 seconds should prevent permanent burn-in

### Debug Mode
Add debug output by uncommenting debug sections in the code:
```cpp
// Uncomment for point count display
// display.print("P:"); display.print(g_pointCount);
```

## Performance Metrics

- **Battery Life**: ~5-7 days with 0.5-second updates and optimized refresh strategy
- **Memory Usage**: ~6KB RTC memory for state persistence
- **CPU Usage**: Minimal during deep sleep, brief activity during updates
- **Display Refresh**: Partial refresh every 0.5 seconds, full refresh every 30 seconds to prevent ghosting

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request. For major changes, please open an issue first to discuss what you would like to change.

### Development Setup
1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly on actual Watchy hardware
5. Submit a pull request

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- **Watchy Team**: For the amazing ESP32 smartwatch platform
- **Edward Lorenz**: For discovering the Lorenz attractor system
- **Arduino Community**: For the excellent development tools
- **ESP32 Community**: For the robust IoT platform

## References

- [Watchy Documentation](https://watchy.sqfmi.com/)
- [Lorenz Attractor](https://en.wikipedia.org/wiki/Lorenz_system)
- [Runge-Kutta Methods](https://en.wikipedia.org/wiki/Runge%E2%80%93Kutta_methods)
- [ESP32 Deep Sleep](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/deep_sleep.html)

---

**Made with love for the Watchy community**

*Watch the beauty of chaos unfold on your wrist*