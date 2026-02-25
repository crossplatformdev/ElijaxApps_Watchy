# Watchy Application SDK

The SDK is a small, static layer over the existing Watchy lifecycle. It keeps
the original wake, RTC, display, radio, and deep-sleep architecture while
giving applications a common UI and input vocabulary.

It deliberately uses stack models, function pointers, and static functions.
There is no virtual application hierarchy and the SDK does not own heap
memory.

This document describes the current firmware contract. The
[documentation index](readme.md) links the measurement, gallery, calibration,
and timing guides that validate individual subsystems.

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
| `ScrollableTextView` | Word-wrapped long text with UP/DOWN paging and a position indicator |
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

Set `ListModel::compactText` to `true` for dense two-column rows. Compact lists
use the built-in 1x font and reserve space between the detail value and the
disclosure arrow; standard lists retain the default SDK typography.

Use `ScrollableTextView::show()` when content may exceed the `200x200`
viewport. It wraps text without allocating per-line storage, pages with UP and
DOWN, and returns when BACK is pressed:

```cpp
WatchyUi::ScrollableTextView::show("ARTICLE", article.c_str());
```

Use `ScrollableTextView::draw()` with a `ScrollableTextModel` when the caller
owns input handling or needs a deterministic render-only preview. As with the
other draw-only views, call `Screen::present()` after composing the screen.

`Screen::present()` uses the one full refresh required after a cold start,
then uses partial refreshes for the rest of that boot and every deep-sleep
wake. Applications do not choose the panel refresh mode.

For dynamic views, redraw only changed content in the retained 1-bit
framebuffer, then invalidate up to four regions:

```cpp
WatchyUi::Screen::invalidate(oldBounds);
WatchyUi::Screen::invalidate(newBounds);
WatchyUi::Screen::presentDirty();
```

The SDK clips and byte-aligns regions for the SSD1681, merges overlapping or
low-cost nearby regions, and accumulates aligned pixel area as refresh debt.
It retains a separate hard partial-refresh count. Dirty presentations always
use partial panel updates; when either limit is due, the full refresh remains
pending until the next complete-screen `Screen::present()`. That full refresh
resets both counters. `Screen::present(bounds)` remains the single-region
convenience API.

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
and then draws a standard title. `Screen::present()` requests a full update
when the driver still has its RTC-backed initial refresh pending, when aligned
dirty-pixel debt reaches its limit, or when the hard partial-refresh count is
due. Otherwise it uses a partial update. Dirty-region presentation never
promotes an incomplete framebuffer to a full-screen update.

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

Use `wait(deadlineMs)` inside fixed-rate sensor or animation loops and `wait()`
for event-driven views. Reserve `poll()` for CPU-bound work that cannot block,
use `pressed()` for controls that intentionally react while held, and use
`waitForRelease()` only when release timing is part of the interaction.

Live sensor views that redraw periodically should use
`WatchyUi::Screen::liveViewRefreshIntervalMs` as their standard cadence. The
interval is `1000 ms`, balancing visible updates against e-paper refresh and
power costs. A synchronous view can redraw, then call
`Input::wait(Screen::liveViewRefreshIntervalMs)` to remain responsive to BACK
without adding background work.

Use `waitScheduled()` while another FreeRTOS task must continue running; it
blocks on button notifications but never enters light sleep. Use
`waitNotified()` when a driver callback, such as Wi-Fi GOT_IP, may notify the
task: it returns for a button, external notification, or deadline, keeping
wake cause separate from button semantics.

`Input::begin()` establishes the active UI task and captures a fresh button
baseline. The common menu dispatcher calls it before every application, and a
full menu render calls it again after returning. Background workers must not
own button interrupts; they use their own events or task primitives. This keeps
input available while heart-rate, metronome, radio, or sensor work is active.

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
   `src/sdk/include/MenuModel.h`.
3. Add `{"Standard Counter", MENU_ACTION_STANDARD_COUNTER}` to the desired
   category in `src/os/ShowMenu.cpp`.
4. Include `app/StandardCounter.h` in `src/sdk/WatchyCore.cpp` and dispatch it
   from `selectMenuEntry()` with:

```cpp
case MENU_ACTION_STANDARD_COUNTER:
  StandardCounterApp::run();
  break;
```

For a family of related tools, prefer one dispatcher with a compact numeric
argument, as used by Time Tools, Sensors, Utilities, and Bluetooth.

## Hardware Ownership

The SDK does not hide Watchy's hardware abstractions. An app that enables Wi-Fi
or BLE must stop it on every exit path and restore the low-power CPU frequency.
Use partial display refreshes for normal interaction and preserve the existing
deep-sleep handoff.

`WatchySensor::SensorManager` is the sole owner of BMA423 ODR, range, APS,
FIFO, watermark, latch mode, and INT2 mapping. Background modes are Baseline,
FallMonitoring, and WatchfaceBcg. Foreground consumers acquire one of
ForegroundHeartRate, ForegroundFall, or LiveAcceleration and release the same
mode on every exit; release reapplies the exact retained background mode.
Applications must not call raw BMA configuration methods directly.

Use `WatchyUi::deepSleepDelay(milliseconds)` instead of Arduino `delay()` for
application timing. On ESP32-S3 it uses timer-driven light sleep when Wi-Fi and
Bluetooth are off, preserving the current call stack and application state. It
returns `WakeupReason::BACK_PRESSED` when BACK interrupts either wait,
`WakeupReason::DEEP_SLEEP_DELAY` after a timer wake, and
`WakeupReason::SCHEDULER_DELAY` when an active radio or sleep error requires an
RTOS task delay. Foreground UI delays should handle `BACK_PRESSED` through the
same cleanup path as `Input::wait()` returning `Event::BACK`. Actual ESP32 deep
sleep remains reserved for the firmware lifecycle because waking from it
resets execution rather than returning from the function call.

`WatchyUi::Power` also owns GPIO waiting. With radios off it uses explicit
Light-sleep. With Wi-Fi or BLE active it blocks the application task on a GPIO
ISR notification so radio connections remain under the IDF modem-sleep policy;
it does not poll every `10 ms`. E-paper BUSY waiting uses bounded 50 ms slices
so the display driver's timeout remains authoritative if the panel never
signals ready. BACK is retained as an input event during those slices, so a
missed BUSY edge cannot make an application unresponsive.

The configured Watchy battery is `200 mAh`. Battery and runtime tools must derive
capacity estimates from `WATCHY_DEFAULT_BATTERY_CAPACITY_MAH` rather than
embedding a separate capacity value. Use `WatchyBattery::estimate()` from
`app/BatteryModel.h` for voltage, estimated charge, and percentage. Its shared
contract maps `WATCHY_BATTERY_EMPTY_VOLTAGE` (`2.65 V`) to `0%` and
`WATCHY_BATTERY_FULL_VOLTAGE` (`3.95 V`) to
`WATCHY_DEFAULT_BATTERY_CAPACITY_MAH` (`200 mAh`) and `100%`; battery views
should show the estimate as `current mAh / capacity mAh` so the configured
capacity remains visible.

The 7-SEG WatchFace's experimental BCG reading uses the BMA423 accelerometer
FIFO at an effective `25 Hz`. The sensor collects samples while the ESP32 is in
deep sleep. A `900-byte` FIFO watermark on `ACC_INT_2` wakes the ESP32 after
`150` samples, approximately every `6 s`; the remaining `120 bytes` provide
`20` samples or `0.8 s` of service margin before the `1024-byte` FIFO fills.
The fixed five-second ESP32 timer is not used. One average is published per
`15 s` window. Watermark wakes that do not complete a window do not initialize
the display. CPU tasks do not continue executing in deep sleep. Leaving the
WatchFace normally disables this FIFO, except while fall monitoring owns it for
bounded pre-trigger capture. `SensorManager` serializes both owners.

Fall monitoring takes priority over watchface BCG monitoring because both
features require the single BMA423 FIFO. Enabling background fall logging
therefore suspends BCG collection until logging is disabled or its bounded
trace buffer becomes full.

## Power Diagnostics

The optional `power-diagnostics` PlatformIO environment enables RTC-retained
counters without writing NVS or Flash. On each wake it emits the completed
history as one `@WATCHY_POWER` line at `115200` baud. Counters include wake
causes, awake/light-sleep milliseconds, display initializations, full/partial
refreshes, aligned dirty pixels and full-screen equivalents, BCG
services/samples/results/visible changes, worker timeouts, Wi-Fi/BLE sessions
and radio-on milliseconds, sensor wakes, heart-task stack high-water mark,
metronome beats/skipped slots/maximum deadline lateness/stack headroom, and
minimum heap. Networking also records free heap and largest contiguous block
before radio, connected, downloaded, parsed, and after shutdown, plus maximum
session heap loss. Production builds compile these hooks to no-ops.

Cadence-sensitive applications use the shared Metronome worker rather than
emitting vibration from their render/input loop. It advances microsecond
deadlines from the prior deadline, retains fractional period remainders, and
skips stale slots after a late wake. Its 2,048-byte task stack is allocated only
while Musical Metronome or CPR Metronome is active. See
[Metronome Timing](metronome-timing.md) for the host and physical validation
protocol.

The deterministic gallery captures every WatchFace in light and dark polarity
with fixed fixtures. `tools/watchface_golden_hashes.json` is independent of
the generated manifest, so a gallery recapture cannot silently approve a
WatchFace pixel change.

```text
pio run -e power-diagnostics -t upload --upload-port COM3
pio device monitor --port COM3 --baud 115200
```

Build the complete firmware after adding an app:

```text
pio run
```