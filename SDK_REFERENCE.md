# Watchy UiSDK Reference

**Version 1.0** — Comprehensive API reference for the Watchy unified display framework

---

## Table of Contents

1. [Overview](#overview)
2. [Core Concepts](#core-concepts)
3. [Display Wrapper Functions](#display-wrapper-functions)
4. [UI Spec Structures](#ui-spec-structures)
5. [Rendering Functions](#rendering-functions)
6. [Theme System](#theme-system)
7. [Font Management](#font-management)
8. [Input Helpers](#input-helpers)
9. [Best Practices](#best-practices)
10. [Examples](#examples)

---

## Overview

The **UiSDK** is a unified display abstraction layer for Watchy firmware. It provides:

- **Consistent API**: All watchfaces and apps use the same display functions via `UiSDK::` namespace
- **Theme Support**: Automatic light/dark mode with polarity-aware bitmap inversion
- **Spec-Based Rendering**: Declarative UI definition using structured specs (text, images, menus, buttons, etc.)
- **Display Wrapper Functions**: Direct pass-through to `GxEPD2` display primitives with consistent semantics

The SDK sits between your watchface/app code and the underlying e-paper display driver (`ThemeableGxEPD2_BW`), applying theme transformations and providing higher-level UI primitives.

---

## Core Concepts

### Display Object

All drawing happens through a **display object** passed to your draw functions:

```cpp
void drawWatchface(WatchyGxDisplay &display) {
    UiSDK::fillScreen(display, GxEPD_WHITE);
    UiSDK::setFont(display, &FreeMonoBold9pt7b);
    UiSDK::setCursor(display, 10, 100);
    UiSDK::setTextColor(display, GxEPD_BLACK);
    UiSDK::print(display, "Hello!");
}
```

**Type alias**: `WatchyGxDisplay = ThemeableGxEPD2_BW<WatchyDisplay, WatchyDisplay::HEIGHT>`

### Polarity System

Every watchface has an **authored polarity** (how it was originally designed):

- **`WatchfacePolarity::WhiteOnBlack`**: White ink on black paper (e.g., 7_SEG, DOS, StarryHorizon)
- **`WatchfacePolarity::BlackOnWhite`**: Black ink on white paper (e.g., Pokemon, Tetris, MacPaint)

The **global theme** (`gDarkMode`) can invert this at runtime:
- **Dark mode enabled** → renders as white-on-black
- **Light mode enabled** → renders as black-on-white

The SDK automatically computes whether bitmaps/colors need inversion based on:
1. Authored polarity (`UiSDK::setPolarity()`)
2. Current global theme (`BASE_POLARITY` macro)

---

## Display Wrapper Functions

### Screen Setup

```cpp
void UiSDK::init(WatchyGxDisplay &display, uint32_t serial_diag_bitrate = 0, 
                 bool initial = true, uint16_t reset_duration = 20, 
                 bool pulldown_rst_mode = false);
```
Initialize the display hardware.

```cpp
void UiSDK::setFullWindow(WatchyGxDisplay &display);
```
Set drawing window to full screen (200x200 pixels).

```cpp
void UiSDK::setRotation(WatchyGxDisplay &display, uint8_t r);
```
Set screen rotation (0-3). Default is 0.

```cpp
void UiSDK::hibernate(WatchyGxDisplay &display);
```
Put display into low-power sleep mode.

```cpp
void UiSDK::displayUpdate(WatchyGxDisplay &display, bool full = true);
```
Push framebuffer to e-paper panel. Set `full=false` for partial refresh (faster, but can cause ghosting).

### Screen Dimensions

```cpp
int16_t UiSDK::width(WatchyGxDisplay &display);    // Returns 200
int16_t UiSDK::height(WatchyGxDisplay &display);   // Returns 200
```

### Drawing Primitives

#### Pixels

```cpp
void UiSDK::drawPixel(WatchyGxDisplay &display, int16_t x, int16_t y, uint16_t color);
void UiSDK::writePixel(WatchyGxDisplay &display, int16_t x, int16_t y, uint16_t color);
```

#### Fill Operations

```cpp
void UiSDK::fillScreen(WatchyGxDisplay &display, uint16_t color);
void UiSDK::fillRect(WatchyGxDisplay &display, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void UiSDK::fillRoundRect(WatchyGxDisplay &display, int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color);
void UiSDK::fillCircle(WatchyGxDisplay &display, int16_t x, int16_t y, int16_t r, uint16_t color);
void UiSDK::fillTriangle(WatchyGxDisplay &display, int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
```

#### Outlines

```cpp
void UiSDK::drawRect(WatchyGxDisplay &display, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void UiSDK::drawRoundRect(WatchyGxDisplay &display, int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color);
void UiSDK::drawCircle(WatchyGxDisplay &display, int16_t x, int16_t y, int16_t r, uint16_t color);
void UiSDK::drawTriangle(WatchyGxDisplay &display, int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
```

#### Lines

```cpp
void UiSDK::drawLine(WatchyGxDisplay &display, int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
void UiSDK::drawFastHLine(WatchyGxDisplay &display, int16_t x, int16_t y, int16_t w, uint16_t color);
void UiSDK::drawFastVLine(WatchyGxDisplay &display, int16_t x, int16_t y, int16_t h, uint16_t color);
```

### Bitmaps

```cpp
void UiSDK::drawBitmap(WatchyGxDisplay &display, int16_t x, int16_t y, 
                       const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color);

void UiSDK::drawBitmap(WatchyGxDisplay &display, int16_t x, int16_t y, 
                       const uint8_t *bitmap, int16_t w, int16_t h, 
                       uint16_t color, uint16_t bg);
```

Draws a monochrome bitmap. Bitmaps should be stored as horizontal byte arrays (MSB first).

**Parameters**:
- `x, y`: Top-left corner
- `bitmap`: Pointer to bitmap data (can be in PROGMEM)
- `w, h`: Width and height in pixels
- `color`: Color for set bits (1 pixels)
- `bg`: Background color for unset bits (0 pixels) — only used in 2-argument version

### Text Rendering

#### Font Selection

```cpp
void UiSDK::setFont(WatchyGxDisplay &display, const GFXfont *font);
void UiSDK::setFont(WatchyGxDisplay &display);  // Reset to default font
```

#### Text Configuration

```cpp
void UiSDK::setCursor(WatchyGxDisplay &display, int16_t x, int16_t y);
int16_t UiSDK::getCursorX(WatchyGxDisplay &display);

void UiSDK::setTextColor(WatchyGxDisplay &display, uint16_t c);
void UiSDK::setTextColor(WatchyGxDisplay &display, uint16_t c, uint16_t bg);

void UiSDK::setTextSize(WatchyGxDisplay &display, uint8_t s);
void UiSDK::setTextWrap(WatchyGxDisplay &display, bool w);
```

#### Text Output

```cpp
size_t UiSDK::print(WatchyGxDisplay &display, const T &value);
size_t UiSDK::println(WatchyGxDisplay &display, const T &value);
```

Supports `String`, `const char*`, integers, floats, etc.

#### Text Measurement

```cpp
void UiSDK::getTextBounds(WatchyGxDisplay &display, const T &text, 
                          int16_t x, int16_t y, 
                          int16_t *x1, int16_t *y1, 
                          uint16_t *w, uint16_t *h);
```

Returns the bounding box of text at position `(x,y)`.

---

## UI Spec Structures

Instead of imperative drawing calls, you can define UI layouts declaratively using **spec structures**.

### UITextSpec

```cpp
struct UITextSpec {
    int16_t x, y;           // Position
    int16_t w, h;           // Size (0 = auto)
    const GFXfont *font;
    bool fillBackground;    // Fill bg before drawing?
    bool invert;            // Invert colors vs theme
    String text;
};
```

### UIImageSpec

```cpp
struct UIImageSpec {
    const uint8_t *bitmap;
    int16_t x, y, w, h;
    bool fromProgmem;
    bool fillBackground;
    WatchfacePolarity polarityAuthored;  // How bitmap was designed
};
```

### UIMenuSpec

```cpp
struct UIMenuItemSpec {
    String label;
};

struct UIMenuSpec {
    int16_t x, y, w, h;
    int16_t itemHeight;
    const GFXfont *font;
    const UIMenuItemSpec *items;
    uint8_t itemCount;
    int8_t selectedIndex;   // -1 = none
    uint8_t startIndex;     // First visible item (for scrolling)
    uint8_t visibleCount;   // Max visible rows (0 = all)
};
```

### UIButtonSpec

```cpp
struct UIButtonSpec {
    int16_t x, y, w, h;
    const GFXfont *font;
    String label;
    bool selected;          // Highlighted state
};
```

### UICheckboxSpec

```cpp
struct UICheckboxSpec {
    int16_t x, y;
    const GFXfont *font;
    bool fillBackground;
    String label;
    bool checked;           // [x] vs [ ]
};
```

### UIScrollableTextSpec

```cpp
struct UIScrollableTextSpec {
    int16_t x, y, w, h;
    const GFXfont *font;
    bool fillBackground;
    String text;            // Multi-line text (\n-separated)
    const String *textRef;  // Optional: reference to large document
    uint16_t firstLine;     // Start line for scrolling
    uint8_t maxLines;       // Lines to render
    int16_t lineHeight;     // Pixels between lines
    bool centered;          // Center lines horizontally
    bool wrapLongLines;     // Wrap text that exceeds width
    bool wrapContinuationAlignRight;  // Right-align wrapped continuation
};
```

### UICallbackSpec

```cpp
struct UICallbackSpec {
    void (*draw)(WatchyGxDisplay &display, void *userData);
    void *userData;
};
```

Custom drawing callback for advanced rendering.

### UIAppSpec

Top-level container for a complete screen layout:

```cpp
struct UIAppSpec {
    const UITextSpec *texts;
    uint8_t textCount;
    
    const UIImageSpec *images;
    uint8_t imageCount;
    
    const UIMenuSpec *menus;
    uint8_t menuCount;
    
    const UIButtonSpec *buttons;
    uint8_t buttonCount;
    
    const UICheckboxSpec *checkboxes;
    uint8_t checkboxCount;
    
    const UIScrollableTextSpec *scrollTexts;
    uint8_t scrollTextCount;
    
    const UICallbackSpec *callbacks;
    uint8_t callbackCount;
    
    UIControlsRowLayout controls[4];
    uint8_t controlCount;
    
    UIAppChromeSpec chrome{};
};
```

---

## Rendering Functions

### Spec Renderers

```cpp
void UiSDK::renderText(WatchyGxDisplay &display, const UITextSpec &spec);
void UiSDK::renderImage(WatchyGxDisplay &display, const UIImageSpec &spec);
void UiSDK::renderMenu(WatchyGxDisplay &display, const UIMenuSpec &spec);
void UiSDK::renderButton(WatchyGxDisplay &display, const UIButtonSpec &spec);
void UiSDK::renderCheckbox(WatchyGxDisplay &display, const UICheckboxSpec &spec);
void UiSDK::renderScrollableText(WatchyGxDisplay &display, const UIScrollableTextSpec &spec);
void UiSDK::renderCallback(WatchyGxDisplay &display, const UICallbackSpec &spec);
```

### Full-App Rendering

```cpp
void UiSDK::renderApp(Watchy &watchy, const UIAppSpec &app);
void UiSDK::renderWatchfaceSpec(Watchy &watchy, const UIAppSpec &app);
```

Renders all elements in a `UIAppSpec` in order:
1. Images
2. Texts
3. Menus
4. Buttons
5. Checkboxes
6. Scrollable texts
7. Callbacks
8. Controls row (if `chrome.drawControlsRow` is true)

---

## Theme System

### Polarity Functions

```cpp
void UiSDK::setPolarity(WatchfacePolarity polarity);
WatchfacePolarity UiSDK::getPolarity();
WatchfacePolarity UiSDK::getThemePolarity();
WatchfacePolarity UiSDK::polarityForFaceId(uint8_t id);
WatchfacePolarity UiSDK::liveThemePolarity();
```

### Color Helpers

```cpp
uint16_t UiSDK::getWatchfaceBg(WatchfacePolarity polarity);
uint16_t UiSDK::getWatchfaceFg(WatchfacePolarity polarity);
bool UiSDK::shouldInvertBitmap(const Palette &palette);
```

### Using the Theme System

**In your watchface draw function:**

```cpp
void MyWatchface::draw(Watchy &watchy) {
    // Tell SDK how this watchface was authored
    UiSDK::setPolarity(WatchfacePolarity::BlackOnWhite);
    
    auto &display = watchy.display;
    UiSDK::setFullWindow(display);
    
    // Get theme colors (auto-inverts if needed)
    uint16_t bg = UiSDK::getWatchfaceBg(BASE_POLARITY);
    uint16_t fg = UiSDK::getWatchfaceFg(BASE_POLARITY);
    
    UiSDK::fillScreen(display, bg);
    UiSDK::setTextColor(display, fg);
    // ... rest of drawing code
}
```

**Macro**: `BASE_POLARITY` evaluates to current theme polarity at runtime.

---

## Font Management

### Built-in Fonts

```cpp
const GFXfont *UiSDK::defaultFont();        // FreeMonoBold9pt7b
const GFXfont *UiSDK::tinyFont5x7();        // 5x7 bitmap font
const GFXfont *UiSDK::tinyFont6x8();        // 6x8 proportional bitmap font
const GFXfont *UiSDK::tinyMono6x8();        // 6x8 monospace bitmap font
```

### Font Registry

Register custom fonts for reuse across your app:

```cpp
bool UiSDK::registerFont(uint8_t slot, const GFXfont *font);
const GFXfont *UiSDK::getFont(uint8_t slot);
```

**Slots**:
- Slot 0: `FreeMonoBold9pt7b` (default)
- Slot 1: `tinyFont6x8`
- Slot 2: `tinyMono6x8`
- Slot 3: `tinyFont5x7`
- Slots 4-63: Available for custom fonts

**Example**:

```cpp
#include "sdk/Fonts.h"

void setup() {
    UiSDK::registerFont(10, &FreeSerif18pt7b);
    UiSDK::registerFont(11, &FreeMono12pt7b);
}

void draw() {
    UiSDK::setFont(display, UiSDK::getFont(10));  // FreeSerif18pt
}
```

---

## Input Helpers

### Button Reading

```cpp
bool UiSDK::buttonPressed(uint8_t pin, uint32_t debounceMs = 80, bool activeLow = true);
```

Debounced button state with automatic latch (returns `true` only once per press until released).

**Standard Watchy Pins**:
- `MENU_BTN_PIN` (26): Menu button (top-left)
- `BACK_BTN_PIN` (25): Back button (bottom-left)
- `UP_BTN_PIN` (32): Up button (top-right)
- `DOWN_BTN_PIN` (4): Down button (bottom-right)

**Example**:

```cpp
if (UiSDK::buttonPressed(MENU_BTN_PIN)) {
    // Menu button was pressed
}
```

### Controls Row

```cpp
struct UIControlsRowLayout {
    const char *label;
    void (Watchy::*callback)();
};

void UiSDK::renderControlsRow(Watchy &watchy, const UIControlsRowLayout layout[4]);
```

Draws a standard button hint row (e.g., "BACK", "UP", "OK", "DOWN").

---

## Best Practices

### 1. Always Set Polarity

```cpp
void MyWatchface::draw(Watchy &watchy) {
    UiSDK::setPolarity(WatchfacePolarity::BlackOnWhite);  // <-- Important!
    // ... rest of code
}
```

This ensures bitmaps and colors invert correctly when theme changes.

### 2. Use Theme Colors

```cpp
uint16_t bg = UiSDK::getWatchfaceBg(BASE_POLARITY);
uint16_t fg = UiSDK::getWatchfaceFg(BASE_POLARITY);
UiSDK::fillScreen(display, bg);
UiSDK::setTextColor(display, fg);
```

Never hardcode `GxEPD_WHITE` / `GxEPD_BLACK` directly — use theme helpers.

### 3. Store Bitmaps in PROGMEM

```cpp
const uint8_t PROGMEM myBitmap[] = { /* ... */ };

UIImageSpec img = {
    .bitmap = myBitmap,
    .x = 10, .y = 10, .w = 32, .h = 32,
    .fromProgmem = true,  // <-- Important for PROGMEM bitmaps
    .fillBackground = false,
    .polarityAuthored = WatchfacePolarity::BlackOnWhite
};
```

### 4. Minimize Display Updates

```cpp
UiSDK::setFullWindow(display);
// ... draw everything ...
UiSDK::displayUpdate(display, true);  // Only call once at end
```

Each `displayUpdate()` triggers a full e-paper refresh (~1-2 seconds).

### 5. Use Specs for Complex Layouts

Instead of manual positioning:

```cpp
UITextSpec timeText = {
    .x = 50, .y = 100, .w = 0, .h = 0,
    .font = &FreeMonoBold24pt7b,
    .fillBackground = false,
    .invert = false,
    .text = "12:34"
};

UIImageSpec logoImg = {
    .bitmap = logo,
    .x = 10, .y = 10, .w = 48, .h = 48,
    .fromProgmem = true,
    .fillBackground = false,
    .polarityAuthored = WatchfacePolarity::BlackOnWhite
};

UIAppSpec app = {
    .images = &logoImg, .imageCount = 1,
    .texts = &timeText, .textCount = 1,
    // ... other elements ...
};

UiSDK::renderWatchfaceSpec(watchy, app);
```

---

## Examples

### Minimal Watchface

```cpp
#include "sdk/UiSDK.h"

class SimpleWatchface : public Watchface {
public:
    void draw(Watchy &watchy) override {
        UiSDK::setPolarity(WatchfacePolarity::BlackOnWhite);
        
        auto &display = watchy.display;
        uint16_t bg = UiSDK::getWatchfaceBg(BASE_POLARITY);
        uint16_t fg = UiSDK::getWatchfaceFg(BASE_POLARITY);
        
        UiSDK::setFullWindow(display);
        UiSDK::fillScreen(display, bg);
        
        UiSDK::setFont(display, &FreeMonoBold24pt7b);
        UiSDK::setTextColor(display, fg);
        UiSDK::setCursor(display, 30, 100);
        
        char timeStr[6];
        sprintf(timeStr, "%02d:%02d", watchy.currentTime.Hour, watchy.currentTime.Minute);
        UiSDK::print(display, timeStr);
        
        UiSDK::displayUpdate(display);
    }
};
```

### Using Specs

```cpp
void SimpleWatchface::draw(Watchy &watchy) {
    UiSDK::setPolarity(WatchfacePolarity::BlackOnWhite);
    
    char timeStr[6];
    sprintf(timeStr, "%02d:%02d", watchy.currentTime.Hour, watchy.currentTime.Minute);
    
    UITextSpec timeText = {
        .x = 30, .y = 100,
        .w = 0, .h = 0,
        .font = &FreeMonoBold24pt7b,
        .fillBackground = false,
        .invert = false,
        .text = String(timeStr)
    };
    
    UIAppSpec app = {
        .texts = &timeText,
        .textCount = 1
    };
    
    app.chrome.beginFullScreen = true;
    app.chrome.drawControlsRow = false;
    
    UiSDK::renderWatchfaceSpec(watchy, app);
}
```

### Button Handling

```cpp
void MyApp::run(Watchy &watchy) {
    auto &display = watchy.display;
    
    while (true) {
        UiSDK::fillScreen(display, GxEPD_WHITE);
        UiSDK::setFont(display);
        UiSDK::setTextColor(display, GxEPD_BLACK);
        
        UiSDK::setCursor(display, 10, 50);
        UiSDK::print(display, "Press MENU to exit");
        UiSDK::displayUpdate(display);
        
        if (UiSDK::buttonPressed(MENU_BTN_PIN)) {
            break;  // Exit app
        }
        
        if (UiSDK::buttonPressed(UP_BTN_PIN)) {
            // Handle UP
        }
        
        delay(50);
    }
}
```

---

## Constants

### Colors

```cpp
#define GxEPD_BLACK 0x0000
#define GxEPD_WHITE 0xFFFF
```

### Display Dimensions

```cpp
WatchyDisplay::WIDTH  = 200
WatchyDisplay::HEIGHT = 200
```

### Button Pins

```cpp
#define MENU_BTN_PIN 26    // Top-left
#define BACK_BTN_PIN 25    // Bottom-left
#define UP_BTN_PIN   32    // Top-right
#define DOWN_BTN_PIN 4     // Bottom-right
```

---

## Advanced Topics

### Custom Polarity Table

If adding a new watchface, update `UiSDK::polarityForFaceId()` in [src/sdk/UiSDK.cpp](src/sdk/UiSDK.cpp):

```cpp
WatchfacePolarity UiSDK::polarityForFaceId(uint8_t id) {
    switch (id) {
        case 99:  // MyNewWatchface
            return WatchfacePolarity::WhiteOnBlack;
        // ... existing cases ...
    }
}
```

### Direct Display Access

If you need low-level control, you can bypass UiSDK wrappers and call `display.*` directly:

```cpp
display.fillScreen(GxEPD_WHITE);
display.setFont(&FreeSans12pt7b);
display.println("Raw display call");
```

However, this **bypasses theme inversion** and should only be used for specialized rendering.

---

## See Also

- [examples/HelloWorldApp/](examples/HelloWorldApp/) — Complete example app
- [src/watchfaces/](src/watchfaces/) — 76+ watchfaces using UiSDK
- [THEME_SYSTEM.md](THEME_SYSTEM.md) — Deep dive into polarity/inversion logic
- [FIRMWARE_MANUAL.md](FIRMWARE_MANUAL.md) — User-facing firmware guide

---

**Last Updated**: 2025-01-XX  
**License**: GPLv3  
**Maintainer**: Elías A. Angulo Klein ([crossplatformdev](https://github.com/crossplatformdev))
