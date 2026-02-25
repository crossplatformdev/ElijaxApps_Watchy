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

On USB power, the lifecycle stays awake and services input with
`waitScheduled(100)`. It also checks minute changes. Before deep sleep it
stops foreground work, waits for the display, and configures timer, USB, and
button/sensor wake sources. Already-active wake levels cause configuration to
fail rather than immediately waking again. On failure, runtime GPIOs are
restored and input remains serviced in a retry loop instead of returning to
the empty Arduino `loop()`. Retained menu state is normalized at startup.

1. Initialize semantic input with `WatchyUi::Input::begin()`.
2. Build a model on the stack and draw a standard view.
3. Present it with `WatchyUi::Screen::present()`.
4. Handle `MENU`, `BACK`, `UP`, and `DOWN` events.
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
| `Input` | Debounced press events, non-consuming held state, bounded waits, and release waiting |
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

`Screen::present(nextGuiState, allowPeriodicFullRefresh)` submits the complete
framebuffer to the driver for comparison. It performs the initial full refresh. The default for
`allowPeriodicFullRefresh` is `false`; callers must explicitly pass `true` to
permit periodic full refreshes. Watch-face presentation also passes `false`.
This suppresses periodic full refreshes but not the required initial refresh.

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
pending until a complete-screen `Screen::present()` explicitly allows it. That full refresh
resets both counters. `Screen::present(bounds)` remains the single-region
convenience API.

### Display Contract

Every complete screen starts and finishes through `Screen`:

```cpp
WatchyUi::Screen::beginCanvas();
// Draw text, shapes, and monochrome bitmaps with Theme::foreground().
WatchyUi::Screen::present();
```

`Screen::beginCanvas()` clears dirty regions, loads the persisted theme,
selects the full display window and rotation zero, fills it with
`Theme::background()`, and resets the font, cursor `(0, 0)`, text size `1`,
text wrapping (off), and text color (`Theme::foreground()`). Do not duplicate
this setup in apps, OS apps, or watch faces. `Screen::begin(title)` performs the same setup
and then draws a standard title. `Screen::present()` requests a full update
when the driver still has its RTC-backed initial refresh pending, when aligned
dirty-pixel debt reaches `59 * 200 * 200` pixels, or when the partial-refresh
count reaches `240`, provided periodic refreshes are allowed. Otherwise it
uses a partial update. `present()` does not infer unchanged content from an
empty dirty-region list. `presentDirty()` uses `displayWindow()` for each
region and does not request a periodic full-screen refresh. The driver's
initial refresh requirement still applies, so draw and present a complete
screen before using delta-only updates.

Canvas setup and widget drawing only modify RAM; presentation sends the
completed composition, not an intermediate empty background. For ordinary
non-inverted, non-mirrored full-framebuffer writes and windows, the driver
compares bytes against a 5,000-byte RAM copy. Identical regions skip both SPI
image writes and panel refresh. Changed regions use their smallest enclosing
byte-aligned rectangle for the write, refresh, and required repeat write.
That rectangle can contain unchanged pixels between changes; the controller
addresses groups of eight horizontal pixels, not individual bits.
The copy is not retained across deep sleep and is invalidated by reset,
hibernation, buffer clears, and unsupported image formats. Without a complete
reference, requested regions are sent normally. Explicit full refreshes are
not suppressed. UI refresh-debt counters still account for requested regions,
so they are conservative rather than measurements of actual transferred area.

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

The explicit-size overload reduces the requested scale, down to size 1, until
the text fits its bounds. Text longer than those bounds even at size 1 still
needs wrapping or abbreviation by its caller. `Screen::begin()` falls back to
a compact built-in title when the standard heading is too wide. Long
`Widget::footer()` text wraps into two lines, with a 64-character capacity at
the standard margins. Empty/warning-state details wrap inside their content
area above the footer.

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

## Live Heart Rate

`measureHeartRateSilently(callback)` starts continuous foreground measurement;
there is no duration parameter or fixed warm-up wait. Stop it with
`stopHeartRateMeasurement()` and wait for worker completion before releasing
the view. The app does this on BACK; trace-capture builds also stop when the
capture completes.

The app uses `WatchyBcg::ResultMode::Live` at a nominal 25 Hz. The processor
retains the latest ten accepted beat-to-beat intervals, replacing the oldest
when a new one arrives. Every sample returns the current rounded BPM, computed
as 60 divided by the mean interval in seconds. It averages the available
intervals during startup; two detected beats provide the first estimate and
eleven detected beats provide ten intervals. There is no additional warm-up.
An estimate is valid only when BPM is greater than 30 and less than 220.
Peaks inside the 125 ms
refractory interval are ignored. A beat gap longer than 2500 ms restarts the
history; about five seconds without a beat clears validity. Changing the
sample rate also resets history. The display retains `-- BPM` while
invalid and updates through dirty regions. There is no hard real-time display
guarantee: sensor access, scheduling, and e-paper refresh add latency.

Both modes return the rolling estimate between updates. In live mode,
`Result::windowComplete` signals a new estimate or invalidation. The default
`ResultMode::Windowed` retains the 15-second update flag cadence for replay
and other default callers, but no longer clears beat history when it publishes. It now reports
the same ten-interval estimate, not a 15-second average. `beatDetected` still
marks individual detected peaks. BCG remains experimental; this smoothing is
not a validation of pulse-detection accuracy.

The peak threshold is `max(0.4, envelope * 1.8)` in filtered raw sample
units, reduced from `max(0.8, envelope * 2.2)` as a provisional sensitivity
adjustment for the reported 75 g aluminum enclosure replacing a roughly
30 g enclosure. This is not a gain derived from the mass ratio. The 25 Hz
sampling, filters, interval limits, and ten-interval average are unchanged.
Lower thresholds can admit more motion and noise peaks. Validation with
enclosure-mounted traces and a simultaneous reference pulse measurement is
still required; synthetic weak-pulse and noise cases do not establish accuracy.

All eight watchfaces use live rolling results as FIFO samples are serviced,
without the 15-second publication wait. Full and partial draws both show
`watchfaceHeartRateBpm()`: the last valid ten-interval average retained in RTC
memory, or `0` before any estimate exists. They never show `--` for heart rate.
Signal loss, FIFO recovery, and monitoring restarts preserve that displayed
number; it may therefore be stale, not a current valid measurement. The
foreground measurement app still displays `-- BPM` when its signal is invalid.

## Input

Applications deal in intent rather than GPIO pins:

```cpp
WatchyUi::Input::begin();
while (true) {
  WatchyUi::Event event = WatchyUi::Input::wait();
  if (event == WatchyUi::Event::BACK) return;
   if (event == WatchyUi::Event::MENU) runAction();
}
```

Use `wait(timeoutMs)` inside fixed-rate sensor or animation loops and `wait()`
for event-driven views. Both consume press events through `poll()`; handle the
returned event instead of polling again for the same press. `poll()` debounces
state changes with a nominal `5 ms` scheduler delay (at least one FreeRTOS
tick), confirms the sampled state, and emits only newly pressed buttons. `wait(0)`
returns `NONE` without polling. `Input::pressed(event)` reads the current held
state of MENU, BACK, UP, or DOWN without consuming events, changing the debounce
baseline, or waiting. It returns `false` for `NONE` or unsupported values.
It is a raw level query, not a debounced press event. Pong retains UP/DOWN
events until its next frame and uses `Input::pressed()` for continuous movement.
Apps must not read button GPIOs directly.
`waitForRelease()` is bounded to `750 ms` and updates the button baseline.

Live sensor views that redraw periodically should use
`WatchyUi::Screen::liveViewRefreshIntervalMs` as their standard cadence. The
interval is `1000 ms`, balancing visible updates against e-paper refresh and
power costs. A synchronous view can redraw, then call
`Input::wait(Screen::liveViewRefreshIntervalMs)` to remain responsive to BACK
without adding background work.

Use `waitScheduled()` while another FreeRTOS task must continue running; it
blocks on button notifications but never enters light sleep.
`waitNotified()` first polls, then delegates to `wait(timeoutMs)`. An external
notification can wake the internal wait, but does not itself cause this API to
return; it continues until a button event or timeout.

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

1. Copy its `.h` into `src/sdk/include/App/` and `.cpp` into `src/app/`.
2. Add `MENU_ACTION_STANDARD_COUNTER` to `MenuAction` in
   [src/sdk/include/App/MenuModel.h](../src/sdk/include/App/MenuModel.h).
3. Add `{"Standard Counter", MENU_ACTION_STANDARD_COUNTER}` to the desired
   category in [src/sdk/os/ShowMenu.cpp](../src/sdk/os/ShowMenu.cpp).
4. Include `StandardCounter.h` in `src/sdk/WatchyCore.cpp` and dispatch it
   from `selectMenuEntry()` with:

```cpp
case MENU_ACTION_STANDARD_COUNTER:
  StandardCounterApp::run();
  break;
```

For a family of related tools, prefer one dispatcher with a compact numeric
argument, as used by Time Tools, Sensors, Utilities, and Bluetooth.

## Hardware Ownership

`WatchySdk::Device` owns the display and RTC. `Watchy::display` and
`Watchy::RTC` are references to those same instances, not separate hardware
objects or framebuffers. Display initialization is lazy; `Screen::present()`
and `presentDirty()` ensure initialization, and direct driver image writes
also guard it. Hibernation powers the panel off before issuing deep sleep.

Menus use `ListView`; app display helpers delegate to `Screen`; watch-face
entry points prepare through `beginCanvas()` and their OS presenter uses
`Screen::present()`. Renderers do not directly call the panel refresh or power
APIs. Custom GFX drawing and font metrics remain available for artwork and
layout. Sensor and charging-status GPIOs are not button input; USB presence
uses `Power::usbPluggedIn()`.

The SDK does not hide Watchy's hardware abstractions. An app that enables Wi-Fi
or BLE must stop it on every exit path and restore the low-power CPU frequency.
Use partial display refreshes for normal interaction and preserve the existing
deep-sleep handoff.

The `WatchySensor` functions implemented in
[SensorManager.cpp](../src/app/SensorManager.cpp) are the sole owner of BMA423 ODR, range, APS,
FIFO, watermark, latch mode, and INT2 mapping. Background modes are Baseline,
FallMonitoring, and WatchfaceBcg. Foreground consumers acquire one of
ForegroundHeartRate, ForegroundFall, or LiveAcceleration and release the same
mode on every exit; release reapplies the exact retained background mode.
Applications must not call raw BMA configuration methods directly.

Use `WatchyUi::deepSleepDelay(milliseconds)` instead of Arduino `delay()` for
application timing. It uses FreeRTOS task delays, not light or deep sleep.
On the registered UI task it checks BACK between delays of at most `10 ms`,
returns `WakeupReason::BACK_PRESSED`, and retains that event for the next poll
for up to `5000 ms`. Other tasks use an uninterrupted scheduler delay.
A zero duration returns `NO_DELAY`; completion returns `SCHEDULER_DELAY`.
`DEEP_SLEEP_DELAY` remains an enum value but is not returned by this helper.
Foreground UI delays should handle `BACK_PRESSED` through the
same cleanup path as `Input::wait()` returning `Event::BACK`. Actual ESP32 deep
sleep remains reserved for the firmware lifecycle because waking from it
resets execution rather than returning from the function call.

`WatchyUi::Power::idle()` uses light sleep on ESP32-S3 when USB is disconnected,
radios are off, and wake configuration succeeds. With USB connected, button
notification waits are capped at `100 ms`; otherwise radio-active waits use
GPIO ISR notifications. A held button uses a nominal `5 ms` scheduler delay
(at least one FreeRTOS tick) to avoid
immediate level-triggered wake loops. The display driver's busy callback yields
with `delay(1)`, leaving GxEPD2's timeout in control. The separate
`Power::waitForDisplayReady(timeoutMs)` helper checks BUSY in slices of at most
`50 ms` before hibernation. Neither display wait captures button edges.

The configured Watchy battery is `200 mAh`. Battery and runtime tools must derive
capacity estimates from `WATCHY_DEFAULT_BATTERY_CAPACITY_MAH` rather than
embedding a separate capacity value. Use `WatchyBattery::estimate()` from
[BatteryModel.h](../src/sdk/include/App/BatteryModel.h) for voltage, estimated charge, and percentage. Its shared
contract maps `WATCHY_BATTERY_EMPTY_VOLTAGE` (`2.65 V`) to `0%` and
`WATCHY_BATTERY_FULL_VOLTAGE` (`3.95 V`) to
`WATCHY_DEFAULT_BATTERY_CAPACITY_MAH` (`200 mAh`) and `100%`; battery views
should show the estimate as `current mAh / capacity mAh` so the configured
capacity remains visible.

Background watch-face BCG uses the BMA423 accelerometer
FIFO at an effective `25 Hz`. The sensor collects samples while the ESP32 is in
deep sleep. A `900-byte` FIFO watermark on `ACC_INT_2` wakes the ESP32 after
`150` samples, approximately every `6 s`; the remaining `120 bytes` provide
`20` samples or `0.8 s` of service margin before the `1024-byte` FIFO fills.
The fixed five-second ESP32 timer is not used. The rolling ten-interval average
updates as beats are processed. Watermark wakes only render when the service reports a visible
change. Cold boot requests monitoring for all faces, not just 7-SEG; actual
availability depends on sensor ownership. Before deep sleep, a non-watch-face
GUI state disables watch-face monitoring. USB runtime is a separate awake
path, so do not assume it has the same FIFO service cadence as deep sleep.
CPU tasks do not continue executing in deep sleep. `SensorManager` serializes
BCG and fall-monitoring ownership.

Fall monitoring takes priority over watchface BCG monitoring because both
features require the single BMA423 FIFO. Enabling background fall logging
therefore suspends BCG collection until logging is disabled or its bounded
trace buffer becomes full.

## Power Diagnostics

Both `Watchy::syncNTP()` and `WatchySdk::syncNTP()` (including their overloads)
close the NTP UDP client, switch Wi-Fi off, stop Bluetooth, end Wi-Fi power
accounting, and restore the CPU to 8 MHz before returning, on success or failure.
Callers must connect Wi-Fi before synchronization and reconnect if they need
network access afterward. Calendar updates remain conditional on NTP success.

The optional `power-diagnostics` PlatformIO environment enables RTC-retained
counters without writing NVS or Flash. `WatchyDiagnostics::beginWake()` is
designed to initialize wake accounting and emit the completed history as a
`@WATCHY_POWER` line at `115200` baud. **Current limitation:** startup does not
call this function, so structured counter reporting and wake accounting are
not operational. Other `@WATCHY_POWER` debug messages are not counter records.
The implemented counter schema includes wake
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