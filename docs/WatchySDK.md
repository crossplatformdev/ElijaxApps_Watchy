# Watchy Application SDK

The SDK is a small, static layer over the existing Watchy lifecycle. It keeps
the original wake, RTC, display, radio, and deep-sleep architecture while
giving applications a common UI and input vocabulary.

It deliberately uses stack models, function pointers, and static functions.
There is no virtual application hierarchy and the SDK does not own heap
memory.

## Lifecycle

An application is invoked by the menu during a button wake. It renders and
handles input synchronously, then returns to the menu. The existing Watchy
lifecycle remains responsible for entering deep sleep.

1. Initialize semantic input with `WatchyUi::Input::begin()`.
2. Build a model on the stack and draw a standard view.
3. Present it with `WatchyUi::Screen::present()`.
4. Handle `SELECT`, `BACK`, `UP`, and `DOWN` events.
5. Stop resources owned by the app and return to the menu.

Do not add background heartbeat work to keep a screen alive. E-paper retains
its image without CPU work.

## UI Components

| API | Use |
| --- | --- |
| `Theme` | Persistent light/dark palette and shared layout metrics |
| `Screen` | Standard title, canvas setup, and partial/full presentation |
| `ListView` | Menus and selectors with bounded, scrolling selection |
| `ValueView` | A prominent value with status, detail, and commands |
| `Widget` | Paragraphs, separators, progress, checks, radios, and toggles |
| `Bounds` / `Canvas` | Top-left bounds for aligned shapes and centered text |
| `Feedback` | Information, errors, confirmations, and transient toasts |
| `Input` | Debounced semantic events plus held-button state |
| `Selector` | Bounded or wrapping numeric and time adjustments |

Drawing a list does not refresh the panel. This lets an app compose additional
content before presenting:

```cpp
const char *items[] = {"RUN", "SETTINGS", "RESET"};
WatchyUi::ListView::draw("MY APP", items, 3, selected);
WatchyUi::Screen::present();
```

`Screen::present()` uses the one full refresh required after a cold start,
then uses partial refreshes for the rest of that boot and every deep-sleep
wake. Applications do not choose the panel refresh mode.

### Display Contract

Every complete screen starts and finishes through `Screen`:

```cpp
WatchyUi::Screen::beginCanvas();
// Draw text, shapes, and monochrome bitmaps with Theme::foreground().
WatchyUi::Screen::present();
```

`Screen::beginCanvas()` loads the persisted theme, selects the full display
window, fills it with `Theme::background()`, and selects
`Theme::foreground()` for text. `Screen::begin(title)` performs the same setup
and then draws a standard title. `Screen::present()` consumes the driver's
RTC-backed initial-refresh flag with `display(false)` once, then calls
`display(true)` thereafter.

Menus and WatchFaces pass through this same lifecycle. A selected control may
deliberately fill with the foreground and draw its label with the background.
After a complete screen has been initialized, a transient overlay or dynamic
sensor region may update a smaller partial window, but it must restore the
full window before returning.

### Coordinate Contract

`Bounds{x, y, width, height}` always uses the top-left corner. When text and a
shape share a visual area, define one `Bounds` value and draw both through
`Canvas`. `Canvas::circle()` converts the box to a center and radius, while
`Canvas::centeredText()` uses `getTextBounds()` to account for the active font
metrics. The overload with `textSize` selects the built-in font; omit
`textSize` after configuring a custom GFX font.

```cpp
WatchyUi::Bounds cell{43, 35, 38, 38};
WatchyUi::Canvas::centeredText(cell, "X", 3,
                               WatchyUi::Theme::foreground());
WatchyUi::Canvas::outline(cell.inset(2),
                          WatchyUi::Theme::foreground());
```

Do not derive a frame or circle from `setCursor()` coordinates. The built-in
font treats cursor `y` as its top edge, custom GFX fonts use a baseline, and
Adafruit GFX circles use a center point. `Bounds` is the SDK boundary that
keeps those coordinate systems from leaking into application layout.

## Input

Applications deal in intent rather than GPIO pins:

```cpp
WatchyUi::Input::begin();
while (true) {
  WatchyUi::Event event = WatchyUi::Input::wait();
  if (event == WatchyUi::Event::BACK) return;
  if (event == WatchyUi::Event::SELECT) runAction();
}
```

Use `poll()` inside sensor or animation loops, `wait()` for event-driven views,
`pressed()` for controls that intentionally react while held, and
`waitForRelease()` only when release timing is part of the interaction.

## Durable State

`WatchySdk::Storage` reads and writes one exact-size NVS value. A durable record
should contain a magic value, its byte size, a schema version, validated data,
and a checksum. Keep the checksum field last and checksum the preceding bytes.

Treat writes as staged commits:

1. Copy live state into a candidate record.
2. Validate and checksum the candidate.
3. Write the complete record under one key.
4. Replace live state only if `Storage::write()` succeeds.

This prevents a failed write from leaving runtime state ahead of durable state.
The complete pattern is implemented in
[`StandardCounter.cpp`](../examples/Apps/StandardCounter/StandardCounter.cpp).

## Registering An App

The registry is compile-time data to minimize RAM and startup work. To add the
counter example to this firmware:

1. Copy its `.h` and `.cpp` files into `src/app/`.
2. Add `MENU_ACTION_STANDARD_COUNTER` to `MenuAction` in
   `src/os/MenuModel.h`.
3. Add `{"Standard Counter", MENU_ACTION_STANDARD_COUNTER}` to the desired
   category in `src/os/ShowMenu.cpp`.
4. Include `app/StandardCounter.h` in `src/Watchy.cpp` and dispatch it with:

```cpp
case MENU_ACTION_STANDARD_COUNTER:
  StandardCounterApp::run(*this);
  break;
```

For a family of related tools, prefer one dispatcher with a compact numeric
argument, as used by Time Tools, Sensors, Utilities, and Bluetooth.

## Hardware Ownership

The SDK does not hide Watchy's hardware abstractions. An app that enables Wi-Fi
or BLE must stop it on every exit path and restore the low-power CPU frequency.
An app that changes BMA configuration must restore the prior configuration.
Use partial display refreshes for normal interaction and preserve the existing
deep-sleep handoff.

The standard Watchy battery is `150 mAh`. Battery and runtime tools must derive
capacity estimates from `WATCHY_DEFAULT_BATTERY_CAPACITY_MAH` rather than
embedding a separate capacity value. Use `WatchyBattery::estimate()` from
`app/BatteryModel.h` for voltage, estimated charge, and percentage. Its shared
contract maps `WATCHY_BATTERY_FULL_VOLTAGE` (`3.95 V`) to
`WATCHY_DEFAULT_BATTERY_CAPACITY_MAH` (`150 mAh`) and `100%`; battery views
should show the estimate as `current mAh / capacity mAh` so the configured
capacity remains visible.

The 7-SEG WatchFace's experimental BCG reading uses the BMA423 accelerometer
FIFO at an effective `25 Hz`. The sensor collects samples while the ESP32 is in
deep sleep; the firmware wakes every `5 s` to drain the `1024-byte` FIFO and
publishes one average per `15 s` window. CPU tasks do not continue executing in
deep sleep. Leaving the WatchFace disables and clears this FIFO so applications
can safely reconfigure the accelerometer.

Build the complete firmware after adding an app:

```text
pio run
```