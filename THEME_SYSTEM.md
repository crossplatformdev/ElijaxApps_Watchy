# Watchface Theme System

## Overview

The ElijaxApps firmware now includes a **centralized theme polarity system** that manages color inversion for all watchfaces based on their intrinsic default appearance. This ensures proper display in both light and dark modes.

## SDK Architecture

### WatchfacePolarity Enum

```cpp
enum class WatchfacePolarity : uint8_t {
  WhiteOnBlack,  // Default is white foreground on black background
  BlackOnWhite   // Default is black foreground on white background
};
```

### SDK Helper Functions (UiSDK.h/cpp)

- **`uint16_t getWatchfaceBg(WatchfacePolarity polarity)`**
  - Returns the correct background color based on face polarity and global `gDarkMode`
  - Automatically swaps colors when theme is inverted

- **`uint16_t getWatchfaceFg(WatchfacePolarity polarity)`**
  - Returns the correct foreground color based on face polarity and global `gDarkMode`
  - Automatically swaps colors when theme is inverted

- **`bool shouldInvertBitmap(WatchfacePolarity polarity)`**
  - Returns whether a bitmap should be inverted for the current theme
  - Logic: `invert = (gDarkMode != wantsDarkMode)`

## Watchface Configuration

### Default Polarities (Based on Reference Screenshots)

| Watchface      | Default Polarity     | Note                              |
|----------------|---------------------|-----------------------------------|
| 7_SEG          | WhiteOnBlack        | Digital segments on black background |
| DOS            | WhiteOnBlack        | Terminal text on black screen     |
| StarryHorizon  | WhiteOnBlack        | Stars and grid on dark space      |
| Pokemon        | BlackOnWhite        | Black sprite on white background  |
| MacPaint       | BlackOnWhite        | Black artwork on white canvas     |
| Tetris         | BlackOnWhite        | Black blocks on white background  |
| Mario          | BlackOnWhite        | Black sprite on white background  |

### Implementation Pattern

Each watchface declares its default polarity as a static constant:

```cpp
// Example: WhiteOnBlack default (7_SEG, DOS, StarryHorizon)
static const WatchfacePolarity POLARITY = WatchfacePolarity::WhiteOnBlack;

void WatchyExample::drawWatchFace() {
  // Use SDK helpers instead of direct THEME_BG/THEME_FG
  display.fillScreen(UiSDK::getWatchfaceBg(POLARITY));
  display.setTextColor(UiSDK::getWatchfaceFg(POLARITY));
  
  // For bitmaps
  UIImageSpec icon{};
  icon.backgroundColor = UiSDK::getWatchfaceBg(POLARITY);
  icon.invert = UiSDK::shouldInvertBitmap(POLARITY);
}
```

## Theme Toggle Behavior

### Default Mode (`gDarkMode = true`)

**WhiteOnBlack faces** (7_SEG, DOS, StarryHorizon):
- Background: BLACK
- Foreground: WHITE
- Bitmap invert: false (no inversion)
- Result: **White on black** (matches default)

**BlackOnWhite faces** (Pokemon, MacPaint, Tetris, Mario):
- Background: BLACK (inverted from default)
- Foreground: WHITE (inverted from default)
- Bitmap invert: true (inverted)
- Result: **White on black** (theme override)

### Light Mode (`gDarkMode = false`)

**WhiteOnBlack faces**:
- Background: WHITE (inverted from default)
- Foreground: BLACK (inverted from default)
- Bitmap invert: true (inverted)
- Result: **Black on white** (theme override)

**BlackOnWhite faces**:
- Background: WHITE
- Foreground: BLACK
- Bitmap invert: false (no inversion)
- Result: **Black on white** (matches default)

## Migration Guide

### Before (Manual Theme Logic)

```cpp
void drawWatchFace() {
  display.fillScreen(THEME_BG);
  display.setTextColor(THEME_FG);
  
  icon.backgroundColor = THEME_BG;
  icon.invert = !gDarkMode;  // ❌ Incorrect for some faces
}
```

### After (SDK-Managed)

```cpp
static const WatchfacePolarity POLARITY = WatchfacePolarity::WhiteOnBlack;

void drawWatchFace() {
  display.fillScreen(UiSDK::getWatchfaceBg(POLARITY));
  display.setTextColor(UiSDK::getWatchfaceFg(POLARITY));
  
  icon.backgroundColor = UiSDK::getWatchfaceBg(POLARITY);
  icon.invert = UiSDK::shouldInvertBitmap(POLARITY);  // ✅ SDK handles logic
}
```

## Benefits

1. **Centralized Logic**: Theme inversion logic lives in the SDK, not scattered across watchfaces
2. **Correct Defaults**: Each watchface displays with its intended default appearance
3. **Automatic Inversion**: Theme toggle correctly inverts all faces relative to their defaults
4. **Type Safety**: Enum-based polarity prevents typos and mistakes
5. **Maintainability**: Future watchfaces just declare polarity and use SDK helpers

## Files Modified

### SDK Core
- `src/sdk/UiSDK.h`: Added `WatchfacePolarity` enum and helper function declarations
- `src/sdk/UiSDK.cpp`: Implemented theme helper functions

### Watchfaces (WhiteOnBlack default)
- `src/watchfaces/7_SEG/Watchy_7_SEG.cpp`
- `src/watchfaces/DOS/Watchy_DOS.cpp`
- `src/watchfaces/StarryHorizon/StarryHorizon.cpp`

### Watchfaces (BlackOnWhite default)
- `src/watchfaces/Pokemon/Watchy_Pokemon.cpp`
- `src/watchfaces/MacPaint/Watchy_MacPaint.cpp`
- `src/watchfaces/Tetris/Watchy_Tetris.cpp`
- `src/watchfaces/Mario/Watchy_Mario.cpp`

## Build Status

✅ **Successful build**: All watchfaces compile correctly with the new SDK theme system.

**Flash usage**: 40.0% (1,335,429 bytes / 3,342,336 bytes)
**RAM usage**: 16.3% (53,480 bytes / 327,680 bytes)

## Testing Checklist

- [ ] Toggle theme in menu ("Invert colors")
- [ ] Verify each watchface displays correctly in **default mode** (gDarkMode = true)
- [ ] Verify each watchface displays correctly in **light mode** (gDarkMode = false)
- [ ] Confirm icons/bitmaps are visible in both modes
- [ ] Check that partial refresh works without artifacts
