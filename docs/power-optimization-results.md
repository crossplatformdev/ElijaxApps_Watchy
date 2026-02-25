# Power Optimization Results

This ledger separates directly measured firmware quantities from physical
energy measurements. Reduced refreshed area or wake opportunity is useful,
but is not presented as measured battery-life improvement.

## Current Build

Measured with the production `esp32-s3-devkitc-1` environment on 2026-08-26:

| Metric | Continuation baseline | Current | Delta |
| --- | ---: | ---: | ---: |
| Static RAM | 84,932 B | 85,084 B | +152 B |
| Flash | 2,040,617 B | 2,050,637 B | +10,020 B |
| Compiler diagnostics | 0 | 0 | 0 |
| Architectural/tool tests | 26 | 68 | increased coverage |

Validation completed on 2026-08-26:

- production, deterministic gallery, and power-diagnostics builds pass;
- all 68 host tests pass;
- the synchronized catalog contains 142 applications and 348 scenes;
- WatchFace pixel compatibility remains protected by 16 independent hashes.

The Flash increase buys central BMA423 ownership, a shared/native BCG replay
pipeline, optional retained trace capture, multi-region dirty presentation,
event-driven Wi-Fi connection waits, a high-resolution haptic cadence worker,
linker-map analysis, and stronger tests. Experimental RTC buffers are excluded
from production.

## Display Work

Areas below include the SSD1681 byte-aligned X bounds where applicable. They
are deterministic geometry measurements, not current measurements.

| Path | Before | Current | Area reduction |
| --- | ---: | ---: | ---: |
| Unchanged 7-SEG BPM result | up to 40,000 px | 0 px | 100% |
| Changed 7-SEG BPM region | 40,000 px | 1,728 px | 95.7% |
| Foreground heart + BPM beat | 17,600 px | 3,264 px | 81.5% |
| Accelerometer, maximum live second | 200,000 px/s | 10,592 px/s | 94.7% |
| Spirit level, dot-only move | 40,000 px | about 208-416 px | about 99% |
| Pong, typical ball + CPU paddle frame | 40,000 px | about 456 px | 98.9% |
| Snake, normal head + tail move | 40,000 px | at most 288 px | 99.3% |
| List selection, same visible window | 40,000 px | 8,800 px | 78.0% |
| Metronome, maximum live second | 40,000 px | at most 3,312 px | 91.7% |

Daily Alarm previously woke and redrew a static screen once per second while
open. It now waits indefinitely for a button notification, changing the idle
path from 3,600 timer opportunities and full-screen updates per hour to zero.

Foreground application delays now arm BACK as a wake source. A BACK press
returns `WakeupReason::BACK_PRESSED` from either light sleep or an RTOS wait,
and remains latched until semantic input consumes it. Brief presses therefore
cannot disappear inside feedback, game, sensor, or haptic delays.

Application entry initializes semantic input at the shared dispatcher, and
each full menu render resets the button baseline. This prevents held or stale
button state from leaking between any of the 142 applications and the menu.

Display BUSY waits run in 50 ms slices with a timer and BACK wake source. The
driver regains control after every slice, so its own bounded panel timeout can
recover from a missed BUSY edge instead of leaving an app unresponsive.

BLE discovery runs asynchronously with a five-second deadline and BACK
cancelation. Wi-Fi Survey polls an asynchronous scan every 100 ms, accepts
BACK, and fails safely after eight seconds. Both paths stop radio work before
returning to the menu.

Refresh debt counts aligned dirty pixels and retains a separate hard partial
refresh limit. Dirty presentations always remain partial, even when either
limit is due, because their framebuffer may contain only the changed regions.
The pending full refresh is serviced by the next complete-screen presentation,
which then resets both counters. This prevents a partial heart-rate update from
flushing an incomplete full-screen buffer and erasing the rest of a WatchFace.

## Application Timing

The musical and CPR metronomes now schedule vibration on a dedicated task from
absolute `esp_timer_get_time()` deadlines. Rendering and input never emit a
beat. Fractional period remainders prevent integer tempo drift, and a delayed
worker skips obsolete slots rather than issuing a catch-up burst. The musical
mode uses aligned normal/accent envelopes; CPR uses a uniform envelope at
110 BPM. All exit paths stop the worker and force the motor low.

A native probe covers all 211 integer tempos from 30 through 240 BPM for 12
simulated hours with zero deadline phase error. This is deterministic schedule
evidence, not a physical jitter measurement. The optional diagnostics build
records actual beat count, skipped slots, maximum deadline lateness, and worker
stack headroom. The hardware protocol and interpretation are documented in
[Metronome Timing](metronome-timing.md).

## Sensor Work

| Policy | Effective rate | Watermark | Wake interval | FIFO margin | Wake opportunities/hour |
| --- | ---: | ---: | ---: | ---: | ---: |
| Current BCG | 25 Hz | 150 samples | 6.0 s | 0.8 s | 600 |
| 12.5 Hz candidate | 12.5 Hz | 150 samples | 12.0 s | 1.6 s | 300 |

The 12.5 Hz row is a mathematical candidate only. Production remains at
25 Hz until labeled human traces pass the criteria in the
[BCG Sample-Rate Study](bcg-rate-study.md). Likewise, baseline step counting
remains at 50 Hz until the physical trials in the
[Step Counter ODR Study](step-odr-study.md) demonstrate acceptable 25 Hz
accuracy.

All ODR, FIFO, APS, interrupt, fall-monitoring, BCG, and foreground
accelerometer transitions now have one owner: `SensorManager`. Foreground
consumers restore the exact retained background mode on every exit.

## Radio And Heap

Saved-network connection now waits on Wi-Fi GOT_IP/DISCONNECTED callbacks,
button notifications, or its deadline instead of checking status every
100 ms. The captive portal retains its 180-second unattended timeout and now
also has a 600-second absolute radio-on limit. WiFiManager's required
non-blocking service loop runs at 20 Hz instead of 100 Hz; the installed
library processes one DNS request and one HTTP client per call and contains no
10 ms service requirement.

The optional diagnostics build records minimum free heap and largest block at:

1. before radio start;
2. connected;
3. downloaded;
4. parsed;
5. after radio shutdown.

It also records maximum session-to-session heap loss. `NetworkAppCommon`
changes must be guided by these fields rather than source-level guesses.

Capture reconnecting serial records as JSON with:

```powershell
python tools/capture_power_diagnostics.py --port COM3 --duration 120 `
	--output measurements/power/session.json
```

## Physical Measurements Pending

The following still require the physical Watchy and, for energy claims, a
current monitor or long-duration battery protocol:

- normal WatchFace average current;
- 7-SEG + BCG average current and observed wakes/hour;
- menu idle current and light-sleep residency;
- Wi-Fi active duration and heap recovery over repeated app runs;
- BLE active duration;
- 25 Hz versus 12.5 Hz BCG accuracy on labeled human traces;
- 50 Hz versus 25 Hz step accuracy;
- e-paper ghosting threshold for refresh debt;
- heart-rate task stack high-water mark before reducing its stack;
- metronome GPIO-onset jitter, skipped beats, and worker stack headroom at
  30, 120, and 240 BPM while the display updates;
- physical vibration-envelope timing for musical normal/accent and CPR pulses.

Until those measurements exist, diagnostics report proxies and the firmware
keeps conservative production policies.