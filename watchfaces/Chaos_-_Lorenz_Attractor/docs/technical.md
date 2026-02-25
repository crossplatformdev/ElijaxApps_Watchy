# Technical Documentation

**Copyright (c) 2024 Ajey Pai Karkala**

This document provides detailed technical information about the Lorenz Attractor Watch Face implementation.

## Architecture Overview

The watch face is built on the Watchy ESP32 platform and implements a real-time Lorenz attractor simulation with persistent state across deep sleep cycles.

### Core Components

```
┌─────────────────┐    ┌──────────────────┐    ┌─────────────────┐
│   Watchy Core   │    │  Lorenz System   │    │  Display Layer  │
│                 │    │                  │    │                 │
│ • RTC Management│◄──►│ • Runge-Kutta    │◄──►│ • E-ink Display │
│ • Deep Sleep    │    │ • 3D Projection  │    │ • Watch Elements│
│ • Wake-up Timer │    │ • State Storage  │    │ • Pattern Render│
└─────────────────┘    └──────────────────┘    └─────────────────┘
```

## Lorenz System Implementation

### Mathematical Model

The Lorenz system is defined by three coupled differential equations:

```
dx/dt = σ(y - x)
dy/dt = x(ρ - z) - y  
dz/dt = xy - βz
```

Where:
- **σ (Sigma)**: 10.0 - Prandtl number
- **ρ (Rho)**: 28.0 - Rayleigh number
- **β (Beta)**: 8/3 - Geometric factor

### Numerical Integration

**Runge-Kutta 4th Order Method**:

```cpp
// k1 = f(t, y)
lorenzDerivative(g_pos, k1);

// k2 = f(t + dt/2, y + k1*dt/2)
for (int i = 0; i < 3; i++) {
  temp[i] = g_pos[i] + k1[i] * DT / 2;
}
lorenzDerivative(temp, k2);

// k3 = f(t + dt/2, y + k2*dt/2)
for (int i = 0; i < 3; i++) {
  temp[i] = g_pos[i] + k2[i] * DT / 2;
}
lorenzDerivative(temp, k3);

// k4 = f(t + dt, y + k3*dt)
for (int i = 0; i < 3; i++) {
  temp[i] = g_pos[i] + k3[i] * DT;
}
lorenzDerivative(temp, k4);

// Update position: y = y + (k1 + 2*k2 + 2*k3 + k4) * dt/6
for (int i = 0; i < 3; i++) {
  g_pos[i] += (k1[i] + 2*k2[i] + 2*k3[i] + k4[i]) * DT / 6;
}
```

**Integration Parameters**:
- **Time Step (DT)**: 0.05
- **Steps per Update**: 5
- **Total Time per Update**: 0.25 time units

## State Persistence

### RTC Memory Storage

Critical state variables are stored in RTC memory to survive deep sleep:

```cpp
RTC_DATA_ATTR float g_pos[3] = {1.0, 1.0, 1.0};           // Current position
RTC_DATA_ATTR float g_trajectory[LORENZ_MAX_POINTS][3];   // Trajectory points (500)
RTC_DATA_ATTR int g_pointCount = 0;                        // Point counter
RTC_DATA_ATTR int g_trajectoryIndex = 0;                   // Circular buffer index
RTC_DATA_ATTR float g_rotationAngle = 0.0;                 // View rotation
RTC_DATA_ATTR int g_updateCounter = 0;                     // Full refresh counter
```

### Memory Layout

| Variable | Type | Size | Purpose |
|----------|------|------|---------|
| `g_pos` | float[3] | 12 bytes | Current Lorenz position |
| `g_trajectory` | float[500][3] | 6,000 bytes | Trajectory history |
| `g_pointCount` | int | 4 bytes | Number of stored points |
| `g_trajectoryIndex` | int | 4 bytes | Circular buffer position |
| `g_rotationAngle` | float | 4 bytes | 3D view rotation |
| `g_updateCounter` | int | 4 bytes | Full refresh counter |

**Total RTC Memory Usage**: ~6,028 bytes

## 3D to 2D Projection

### Rotation Matrix

The 3D Lorenz points are rotated around the Y-axis for dynamic viewing:

```cpp
void project3DTo2D(float point3D[3], float& x2D, float& y2D) {
  float cos_rot = cos(g_rotationAngle);
  float sin_rot = sin(g_rotationAngle);
  
  // Apply Y-axis rotation
  float x_rot = point3D[0] * cos_rot - point3D[2] * sin_rot;
  float y_rot = point3D[1];
  float z_rot = point3D[0] * sin_rot + point3D[2] * cos_rot;
  
  // Project to 2D (orthographic)
  x2D = x_rot;
  y2D = y_rot;
}
```

### Scaling and Centering

Dynamic scaling ensures the pattern fits the display:

```cpp
// Find bounds
float minX = 999, maxX = -999, minY = 999, maxY = -999;
for (int i = 0; i < numPoints; i++) {
  if (screenX[i] < minX) minX = screenX[i];
  if (screenX[i] > maxX) maxX = screenX[i];
  // ... similar for Y
}

// Calculate scale factor
float rangeX = maxX - minX;
float rangeY = maxY - minY;
float scaleX = 80.0 / rangeX;
float scaleY = 80.0 / rangeY;
float scale = (scaleX < scaleY) ? scaleX : scaleY;

// Center and scale
float centerX = (minX + maxX) / 2;
float centerY = (minY + maxY) / 2;
screenX[i] = (screenX[i] - centerX) * scale + 100;
screenY[i] = (screenY[i] - centerY) * scale + 100;
```

## Wake-up and Sleep Management

### Custom Sleep/Wake Management

The watch uses custom `init()` and `deepSleep()` methods for precise wake-up control:

```cpp
void init(String datetime = "") {
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  // Handle different wake-up sources (timer, button, USB)
  // Update display and enter deep sleep
}

void deepSleep() {
  display.hibernate();
  RTC.clearAlarm();
  // Configure wake-up sources (timer, buttons, USB)
  esp_sleep_enable_timer_wakeup(WAKE_UP_INTERVAL_MINUTES * 60.0 * 1000000.0);
  esp_deep_sleep_start();
}
```

### Sleep Cycle

1. **Wake-up**: ESP32 exits deep sleep
2. **Initialize**: RTC memory variables restored
3. **Check Refresh**: Increment counter and determine refresh type (partial/full)
4. **Update Lorenz**: 5 integration steps
5. **Add Points**: Store new trajectory points
6. **Rotate View**: Update 3D rotation angle
7. **Render**: Draw pattern, moon phase, sunrise/sunset, and watch elements
8. **Display**: Partial refresh (fast) or full refresh (every 60 updates)
9. **Sleep**: Enter deep sleep for 0.5 seconds

## Display Management

### E-ink Display

The Watchy uses a 200x200 pixel e-ink display:

- **Resolution**: 200x200 pixels
- **Drawing Area**: 190x190 pixels (5px margins)
- **Color Depth**: 1-bit (black/white)
- **Refresh Strategy**: Partial refresh every 0.5s, full refresh every 30s (configurable)

### Refresh Strategy

To balance smooth animation with display longevity and battery life, the watch uses a hybrid refresh strategy:

**Partial Refresh (Most Updates)**:
- Fast update (~200ms)
- Lower power consumption
- Preserves display lifespan
- May show slight ghosting
- Used for 59 out of 60 updates

**Full Refresh (Every 30 seconds)**:
- Complete display regeneration
- Eliminates ghosting
- Higher power consumption (~400ms)
- Prevents permanent screen burn-in
- Used every 60th update

```cpp
g_updateCounter++;
bool fullRefresh = (g_updateCounter >= FULL_REFRESH_INTERVAL);
if (fullRefresh) {
  g_updateCounter = 0;
}
showWatchFace(fullRefresh);
```

**Configuration**: Adjust `FULL_REFRESH_INTERVAL` in `settings.h`:
- Lower values = more frequent full refreshes (cleaner, more battery drain)
- Higher values = fewer full refreshes (more ghosting, better battery)
- Recommended: 60 updates (30 seconds) for optimal balance

### Drawing Pipeline

1. **Clear Display**: `display.fillScreen(GxEPD_WHITE)`
2. **Draw Time**: 24-hour format at top-left
3. **Draw Moon Phase**: Calculate and render lunar phase indicator
4. **Draw Date**: Positioned to avoid moon overlap
5. **Draw Battery**: Percentage-based indicator (3.0V-4.2V range)
6. **Draw Steps**: Step counter at bottom-left
7. **Calculate Trajectory**: Project 3D points to 2D
8. **Scale and Center**: Fit pattern to display
9. **Draw Points**: Render trajectory as 2x2 squares
10. **Draw Lines**: Connect consecutive points
11. **Draw Sunrise/Sunset**: Calculate and display solar times
12. **Update Display**: `display.display(fullRefresh)` - partial or full based on counter

## Performance Analysis

### Computational Complexity

**Per Update Cycle**:
- **Integration Steps**: 5 × 4 = 20 derivative calculations
- **Point Storage**: 5 array operations
- **3D Projection**: 500 × 3 = 1,500 trigonometric operations
- **Scaling**: 500 × 4 = 2,000 arithmetic operations
- **Drawing**: 500 × 4 = 2,000 pixel operations
- **Moon Phase Calculation**: ~100 arithmetic operations
- **Sunrise/Sunset Calculation**: ~50 trigonometric operations

**Total Operations per Update**: ~5,670 operations

### Memory Usage

| Component | Size | Type | Purpose |
|-----------|------|------|---------|
| Program Code | ~20KB | Flash | Main program |
| RTC Variables | ~6KB | RTC RAM | State persistence |
| Local Variables | ~2KB | RAM | Runtime calculations |
| Display Buffer | ~5KB | RAM | E-ink display |

### Power Consumption

**Active Mode** (during update):
- **CPU**: 240MHz
- **Current**: ~80mA
- **Duration**: ~200ms
- **Energy**: ~16mJ per update

**Sleep Mode**:
- **Current**: ~10μA
- **Duration**: 0.3s
- **Energy**: ~3μJ per cycle

**Total Energy per Update**: ~16mJ (99.98% in sleep)

## Error Handling

### Numerical Stability

**Overflow Protection**:
```cpp
// Clamp to screen bounds
if (screenX[i] < 5) screenX[i] = 5;
if (screenX[i] > 195) screenX[i] = 195;
if (screenY[i] < 5) screenY[i] = 5;
if (screenY[i] > 195) screenY[i] = 195;
```

**Division by Zero**:
```cpp
// Ensure minimum range
if (rangeX < 0.1) rangeX = 0.1;
if (rangeY < 0.1) rangeY = 0.1;
```

### Moon Phase Calculation

The moon phase is calculated using the lunar cycle:

```cpp
float calculateMoonPhase() {
  const float MOON_CYCLE_DAYS = 29.53058867;
  // Calculate UTC seconds from 2000-01-01
  // Apply timezone offset
  // Calculate days since reference new moon
  // Return phase (0.0 = new moon, 0.5 = full moon)
}
```

### Sunrise/Sunset Calculation

Solar times are calculated based on location:

```cpp
void calculateSunriseSunset(float& sunriseHour, float& sunsetHour) {
  // Calculate solar declination from day of year
  // Calculate hour angle from latitude and declination
  // Apply equation of time correction
  // Apply longitude and timezone corrections
  // Return sunrise and sunset times in hours
}
```

## Optimization Techniques

### Circular Buffer

Efficient trajectory storage using modulo arithmetic:

```cpp
if (g_pointCount < MAX_POINTS) {
  // Linear growth phase
  g_trajectory[g_trajectoryIndex][0] = g_pos[0];
  g_trajectoryIndex++;
  g_pointCount++;
} else {
  // Circular buffer phase
  g_trajectory[g_trajectoryIndex][0] = g_pos[0];
  g_trajectoryIndex = (g_trajectoryIndex + 1) % MAX_POINTS;
  g_pointCount = MAX_POINTS;
}
```

### Trigonometric Optimization

Pre-calculate rotation values:

```cpp
float cos_rot = cos(g_rotationAngle);
float sin_rot = sin(g_rotationAngle);
// Reuse for all points in current frame
```

### Memory Alignment

RTC variables aligned for optimal access:

```cpp
RTC_DATA_ATTR float g_pos[3] __attribute__((aligned(4)));
```

## Debugging and Diagnostics

### Debug Output

Enable debug information by uncommenting:

```cpp
// Point count display
display.print("P:"); display.print(g_pointCount);

// Position display  
display.print("X:"); display.print(g_pos[0]);
display.print("Y:"); display.print(g_pos[1]);
display.print("Z:"); display.print(g_pos[2]);
```

### Performance Monitoring

Track update timing:

```cpp
unsigned long startTime = millis();
// ... update code ...
unsigned long updateTime = millis() - startTime;
display.print("T:"); display.print(updateTime);
```

## Additional Features

### Moon Phase Display

- **Calculation**: Based on 29.53-day lunar cycle
- **Reference**: New moon on 2000-01-05 18:14 UTC
- **Timezone Support**: Adjusts for local timezone
- **Visualization**: Circular indicator with phase-based filling
- **Position**: Top-right corner, 10-pixel radius

### Sunrise/Sunset Display

- **Calculation**: Solar elevation angle method
- **Location-Based**: Uses configured latitude/longitude
- **Timezone Support**: Accounts for timezone offset
- **Equation of Time**: Corrects for Earth's orbital eccentricity
- **Display Format**: "HHMM/HHMM" (sunrise/sunset)
- **Position**: Centered at bottom

### Battery Indicator

- **Percentage-Based**: Calculates from 3.0V (0%) to 4.2V (100%)
- **Visual**: Filled rectangle proportional to charge level
- **Position**: Bottom-right corner

## Future Enhancements

### Potential Improvements

1. **Adaptive Scaling**: Dynamic scale factor based on pattern size
2. **Color Depth**: Multi-level grayscale display
3. **Pattern Modes**: Different attractor systems
4. **User Interaction**: Button controls for settings
5. **Data Logging**: Save pattern evolution to flash
6. **Weather Integration**: Display weather data alongside sunrise/sunset

### Hardware Considerations

- **Memory**: Larger trajectory buffers
- **Processing**: Faster wake-up times
- **Display**: Higher resolution e-ink
- **Sensors**: Accelerometer for rotation control

---

**Technical excellence through mathematical precision** 🔬⚡
