# HelloWorldApp Example

A minimal Watchy app demonstrating the UiSDK API.

## What It Does

1. Displays **"Hello World"** in large text (centered)
2. Shows **current time** as `hh:mm:ss` format (centered)
3. Shows an **exit hint**: "MENU = Exit"
4. Respects the **global theme** (dark/light mode)
5. Waits for **MENU button** press to exit

## Code Walkthrough

### 1. Drawing Text

```cpp
UiSDK::setFont(display, &FreeMonoBold18pt7b);
UiSDK::setTextColor(display, fg);
UiSDK::setCursor(display, x, y);
UiSDK::print(display, "Hello World");
```

### 2. Getting Current Time

```cpp
tmElements_t &tm = watchy.currentTime;
char timeStr[12];
sprintf(timeStr, "%02d:%02d:%02d", tm.Hour, tm.Minute, tm.Second);
```

### 3. Theme-Aware Colors

```cpp
uint16_t bg = gDarkMode ? GxEPD_BLACK : GxEPD_WHITE;
uint16_t fg = gDarkMode ? GxEPD_WHITE : GxEPD_BLACK;
UiSDK::fillScreen(display, bg);
UiSDK::setTextColor(display, fg);
```

### 4. Centering Text

```cpp
int16_t x1, y1;
uint16_t w, h;
UiSDK::getTextBounds(display, "Hello World", 0, 0, &x1, &y1, &w, &h);
int16_t centerX = (200 - w) / 2;  // 200 = screen width
UiSDK::setCursor(display, centerX, y);
```

### 5. Button Handling

```cpp
if (UiSDK::buttonPressed(MENU_BTN_PIN)) {
    // Exit app
}
```

`UiSDK::buttonPressed()` handles debouncing and button latch automatically.

## How to Integrate

To add this app to the Watchy firmware:

1. **Add to app registry** in `src/os/AppRegistry.cpp`:
   ```cpp
   extern App *getHelloWorldApp();
   
   App *g_appTable[] = {
       // ... existing apps ...
       getHelloWorldApp(),
   };
   ```

2. **Add menu entry** in `src/apps/MenuApp.cpp`:
   ```cpp
   UIMenuItemSpec menuItems[] = {
       // ... existing items ...
       {"Hello World"},
   };
   ```

3. **Build and flash**:
   ```bash
   platformio run --target upload --environment esp32-s3-devkitc-1
   ```

## Files

- `HelloWorldApp.cpp` — Implementation
- `README.md` — This file

## See Also

- [SDK_REFERENCE.md](../../SDK_REFERENCE.md) — Complete UiSDK API documentation
- [src/sdk/UiSDK.h](../../src/sdk/UiSDK.h) — SDK header
- [src/apps/](../../src/apps/) — Other built-in apps

## License

GPLv3 — See [LICENSE](../../LICENSE)
