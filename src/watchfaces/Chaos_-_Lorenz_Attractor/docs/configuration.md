# Configuration Guide

**Copyright (c) 2024 Ajey Pai Karkala**

This guide explains how to customize the Lorenz Attractor Watch Face to suit your preferences.

## Quick Configuration

All settings are located in `chaos/settings.h`. Edit this file to customize behavior:

```cpp
// Wake up settings
#define WAKE_UP_INTERVAL_MINUTES (0.5/60.0)  // 0.5 seconds

// Lorenz attractor settings
#define LORENZ_POINTS_PER_UPDATE 5           // Points per refresh
#define LORENZ_MAX_POINTS 500                // Trajectory buffer size
#define LORENZ_ROTATION_SPEED 0.0523598776   // Rotation speed (3 degrees per update)

// Display refresh settings
#define FULL_REFRESH_INTERVAL 60             // Full refresh every N updates (60 = 30 seconds)

// Location and timezone settings
#define TIMEZONE_OFFSET_HOURS 1               // Timezone offset from UTC
#define LATITUDE 52.3676                      // Latitude in degrees
#define LONGITUDE 4.9041                      // Longitude in degrees
```

## Wake-up Settings

### Wake-up Frequency

Control how often the watch updates:

```cpp
#define WAKE_UP_INTERVAL_MINUTES (0.5/60.0)   // 0.5 seconds (very fast)
#define WAKE_UP_INTERVAL_MINUTES (3.0/60.0)   // 3 seconds (fast)
#define WAKE_UP_INTERVAL_MINUTES (6.0/60.0)   // 6 seconds (medium)
#define WAKE_UP_INTERVAL_MINUTES (30.0/60.0)  // 30 seconds (slow)
#define WAKE_UP_INTERVAL_MINUTES 1.0          // 1 minute (battery saving)
```

**Recommendations**:
- **0.5 seconds**: Very smooth evolution, higher battery usage
- **3 seconds**: Best for watching evolution, good balance
- **6 seconds**: Good balance of evolution and battery life
- **30 seconds**: Battery saving mode, slower evolution
- **1 minute**: Maximum battery life, minimal evolution

## Lorenz Attractor Settings

### Points Per Update

Control how many new points are added each refresh:

```cpp
#define LORENZ_POINTS_PER_UPDATE 2     // Slow evolution
#define LORENZ_POINTS_PER_UPDATE 5    // Default (balanced)
#define LORENZ_POINTS_PER_UPDATE 10   // Fast evolution
#define LORENZ_POINTS_PER_UPDATE 20   // Very fast evolution
```

**Effects**:
- **Higher values**: Faster pattern evolution, more battery usage
- **Lower values**: Slower evolution, better battery life

### Maximum Points

Control trajectory buffer size:

```cpp
#define LORENZ_MAX_POINTS 200   // Shorter trails
#define LORENZ_MAX_POINTS 300   // Medium trails
#define LORENZ_MAX_POINTS 500   // Default (longer trails)
#define LORENZ_MAX_POINTS 1000  // Very long trails (may exceed RTC memory)
```

**Effects**:
- **Higher values**: Longer, more detailed trails, more memory usage
- **Lower values**: Shorter trails, less memory usage

### Rotation Speed

Control how fast the 3D view rotates:

```cpp
#define LORENZ_ROTATION_SPEED 0.0174533   // 1 degree per update (slow)
#define LORENZ_ROTATION_SPEED 0.0523599   // 3 degrees per update (default)
#define LORENZ_ROTATION_SPEED 0.104720    // 6 degrees per update (fast)
#define LORENZ_ROTATION_SPEED 0.209440    // 12 degrees per update (very fast)
```

**Effects**:
- **Higher values**: Faster rotation, more dynamic view
- **Lower values**: Slower rotation, more stable view

## Display Refresh Settings

### Full Refresh Interval

Control how often full display refreshes occur to prevent e-ink ghosting:

```cpp
#define FULL_REFRESH_INTERVAL 30   // Every 15 seconds (aggressive, cleaner display)
#define FULL_REFRESH_INTERVAL 60   // Every 30 seconds (default, balanced)
#define FULL_REFRESH_INTERVAL 120  // Every 60 seconds (battery saving, may show ghosting)
#define FULL_REFRESH_INTERVAL 240  // Every 120 seconds (maximum battery, more ghosting)
```

**How It Works**:
- Most updates use **partial refresh** (fast, low power, minimal flashing)
- Every Nth update uses **full refresh** (complete regeneration, eliminates ghosting)
- Full refresh counter resets to 0 after each full refresh

**Effects**:
- **Lower values**: More frequent full refreshes
  - Cleaner display with no ghosting
  - Higher battery drain
  - More visible flashing
  - Better for display longevity
  
- **Higher values**: Less frequent full refreshes
  - Some temporary ghosting may appear
  - Better battery life
  - Less visible flashing
  - Smoother animation between full refreshes

**Calculating Intervals**:
- With 0.5-second updates: `FULL_REFRESH_INTERVAL × 0.5 = seconds between full refreshes`
- Examples:
  - 30 = 15 seconds
  - 60 = 30 seconds (recommended)
  - 120 = 60 seconds
  - 240 = 2 minutes

**Recommendations**:
- **Default (60)**: Best balance for most users
- **Ghosting visible**: Lower to 30-40
- **Battery critical**: Increase to 120-180
- **Static display**: Can go up to 240+

## Location and Timezone Configuration

### Timezone Offset

Set your timezone offset from UTC:

```cpp
#define TIMEZONE_OFFSET_HOURS 1    // Amsterdam (CET) - winter time
#define TIMEZONE_OFFSET_HOURS 2    // Amsterdam (CEST) - summer time
#define TIMEZONE_OFFSET_HOURS -5   // New York (EST) - winter time
#define TIMEZONE_OFFSET_HOURS -4   // New York (EDT) - summer time
#define TIMEZONE_OFFSET_HOURS 0    // London (GMT) - winter time
#define TIMEZONE_OFFSET_HOURS 1    // London (BST) - summer time
#define TIMEZONE_OFFSET_HOURS 9    // Tokyo (JST)
#define TIMEZONE_OFFSET_HOURS -8   // Los Angeles (PST) - winter time
#define TIMEZONE_OFFSET_HOURS -7   // Los Angeles (PDT) - summer time
```

**Note**: Remember to update this value when daylight saving time changes!

### Location Coordinates

Set your latitude and longitude for accurate sunrise/sunset calculations:

```cpp
// Amsterdam, Netherlands
#define LATITUDE 52.3676
#define LONGITUDE 4.9041

// New York, USA
#define LATITUDE 40.7128
#define LONGITUDE -74.0060

// London, UK
#define LATITUDE 51.5074
#define LONGITUDE -0.1278

// Tokyo, Japan
#define LATITUDE 35.6762
#define LONGITUDE 139.6503

// Los Angeles, USA
#define LATITUDE 34.0522
#define LONGITUDE -118.2437
```

**Finding Your Coordinates**:
- Use Google Maps: Right-click on your location → Coordinates
- Use online tools: Search "latitude longitude finder"
- Format: Decimal degrees (e.g., 52.3676, not 52°22'03")

## Advanced Configuration

### Lorenz System Parameters

For advanced users, you can modify the Lorenz system parameters in `chaos/chaos.ino`:

```cpp
static constexpr float LORENZ_SIGMA = 10.0;    // Prandtl number
static constexpr float LORENZ_RHO = 28.0;      // Rayleigh number
static constexpr float LORENZ_BETA = 8.0/3.0;  // Geometric factor
static constexpr float DT = 0.05;              // Time step
```

**Parameter Effects**:
- **Sigma (σ)**: Controls the rate of change in x direction
- **Rho (ρ)**: Controls the transition to chaos (critical value ~24.74)
- **Beta (β)**: Controls the rate of change in z direction
- **DT**: Integration time step (smaller = more accurate, slower)

### Display Settings

Modify the display window in `drawTrajectory()`:

```cpp
// Current bounds (190x190 pixels)
if (screenX[i] < 5) screenX[i] = 5;
if (screenX[i] > 195) screenX[i] = 195;
if (screenY[i] < 5) screenY[i] = 5;
if (screenY[i] > 195) screenY[i] = 195;

// Larger window (195x195 pixels)
if (screenX[i] < 2) screenX[i] = 2;
if (screenX[i] > 198) screenX[i] = 198;
if (screenY[i] < 2) screenY[i] = 2;
if (screenY[i] > 198) screenY[i] = 198;
```

## Example Configurations

### Battery Saver Mode

For maximum battery life:

```cpp
#define WAKE_UP_INTERVAL_MINUTES 1.0   // 1 minute updates
#define LORENZ_POINTS_PER_UPDATE 2     // Fewer points
#define LORENZ_MAX_POINTS 200          // Shorter trails
#define LORENZ_ROTATION_SPEED 0.0174533 // Slow rotation (1 degree)
#define FULL_REFRESH_INTERVAL 120      // Full refresh every 2 hours
```

**Expected battery life**: 10-14 days

### Performance Mode

For maximum visual impact:

```cpp
#define WAKE_UP_INTERVAL_MINUTES (0.5/60.0)  // 0.5 second updates
#define LORENZ_POINTS_PER_UPDATE 10          // Many points
#define LORENZ_MAX_POINTS 500                // Long trails
#define LORENZ_ROTATION_SPEED 0.104720       // Fast rotation (6 degrees)
#define FULL_REFRESH_INTERVAL 40             // Full refresh every 20 seconds
```

**Expected battery life**: 3-5 days

### Balanced Mode (Default)

Good balance of performance and battery:

```cpp
#define WAKE_UP_INTERVAL_MINUTES (0.5/60.0)  // 0.5 second updates
#define LORENZ_POINTS_PER_UPDATE 5           // Moderate points
#define LORENZ_MAX_POINTS 500                // Longer trails
#define LORENZ_ROTATION_SPEED 0.0523599      // Moderate rotation (3 degrees)
#define FULL_REFRESH_INTERVAL 60             // Full refresh every 30 seconds
```

**Expected battery life**: 5-7 days

## Custom Configurations

### Creating Your Own Config

1. Copy `chaos/settings.h` to `examples/custom_configs/my_config.h`
2. Modify the values as desired
3. Replace the include in `chaos/chaos.ino`:
   ```cpp
   #include "examples/custom_configs/my_config.h"
   ```
4. Upload the modified code

### Sharing Configurations

To share your configuration:

1. Create a new file in `examples/custom_configs/`
2. Name it descriptively (e.g., `battery_saver.h`)
3. Add a comment explaining the purpose
4. Submit a pull request

## Performance Impact

### Battery Life Estimates

| Update Interval | Points/Update | Max Points | Estimated Battery Life |
|----------------|---------------|------------|----------------------|
| 0.5 seconds    | 5            | 500        | 5-7 days             |
| 3 seconds      | 5            | 500        | 7-10 days            |
| 6 seconds      | 5            | 500        | 10-14 days           |
| 30 seconds     | 5            | 500        | 14-20 days           |
| 1 minute       | 5            | 500        | 20-30 days           |

### Memory Usage

- **RTC Memory**: ~6KB for state variables
- **Flash Memory**: ~20KB for program code
- **RAM**: Minimal during deep sleep

## Troubleshooting Configuration

### Common Issues

**Pattern too fast/slow**:
- Adjust `LORENZ_POINTS_PER_UPDATE`
- Modify `WAKE_UP_INTERVAL_MINUTES`

**Battery drains quickly**:
- Increase `WAKE_UP_INTERVAL_MINUTES`
- Decrease `LORENZ_POINTS_PER_UPDATE`
- Increase `FULL_REFRESH_INTERVAL`

**Screen ghosting/burn-in visible**:
- Decrease `FULL_REFRESH_INTERVAL` for more frequent full refreshes
- Try values between 30-60 for best results

**Moon phase or sunrise/sunset incorrect**:
- Check `TIMEZONE_OFFSET_HOURS` matches your timezone
- Verify `LATITUDE` and `LONGITUDE` are correct
- Remember to update timezone for daylight saving time

**Pattern not visible**:
- Check display bounds in `drawTrajectory()`
- Verify scaling parameters

**Compilation errors**:
- Check syntax in `settings.h`
- Ensure all values are valid numbers

## Best Practices

1. **Start with defaults**: Use the provided balanced configuration
2. **Test changes**: Make one change at a time
3. **Monitor battery**: Check battery life with new settings
4. **Backup configs**: Save working configurations
5. **Document changes**: Note what each setting does

---

**Happy customizing!** ⚙️✨
