# Fall Monitoring Calibration

Background fall monitoring is currently a **calibration logger**, disabled by
default. It does not automatically vibrate, broadcast Medical ID data, or call
emergency services. Those actions remain intentionally gated on real trace
collection and threshold validation.

## Status

Bounded background capture and host export are implemented. Classification,
alerting, emergency broadcasts, and threshold promotion remain intentionally
disabled until labeled traces demonstrate acceptable false-positive and
missed-event behavior.

The BMA423 keeps a 25 Hz acceleration history while the ESP32-S3 sleeps. A
high-slope any-motion event is mapped only to the active-low INT2 pin. INT2
wakes the ESP32, which stores up to 96 pre-trigger samples and 64 post-trigger
samples. Four traces are retained in a bounded NVS buffer; monitoring stops
when that buffer is full to prevent repeated flash writes.

## Watch Controls

Open **Health & Care > Fall Detector**:

- **SELECT** toggles background candidate logging.
- **UP** opens the original foreground live detector.
- **DOWN** confirms and clears all stored calibration traces.
- **BACK** returns to the Healthcare menu.

`ARMED` means INT2 wake and trace capture are active. `LOG FULL` means four
traces are stored and must be exported or cleared before monitoring can resume.

## Collect A Batch

Collect one labeled activity per four-trace batch. Useful batches include:

- normal walking
- running
- vigorous arm swing
- sitting and standing
- watch removal and placement
- device drop onto a padded surface while not worn
- supervised staged falls onto appropriate protective mats

Do not perform unsafe or unsupervised falls for calibration.

1. Upload the normal firmware and reset Watchy:

```powershell
pio run -e esp32-s3-devkitc-1 -t upload --upload-port COM3
```

2. Open Fall Detector and press SELECT until it shows `ARMED`.
3. Perform the same labeled activity until the screen later reports
   `LOG FULL`.
4. Connect USB and leave the Fall Detector screen open.
5. Export the traces, replacing `COM3` and the label as needed:

```powershell
python tools/capture_fall_traces.py `
  --port COM3 `
  --label running `
  --output fall-traces/running
```

6. Inspect the generated CSV files and `manifest.json`.
7. Press DOWN on Watchy to clear the buffer before collecting the next label.

Each CSV contains raw X/Y/Z readings, trigger-relative time, phase (`P` for
pre-trigger and `A` for after-trigger), and vector magnitude in g. The host
tool reports neutral minimum and maximum magnitudes only; it does not classify
an event as a fall.

## Build Defaults

The following compile-time defaults can be overridden in PlatformIO:

- `FALL_MONITORING_DEFAULT_ENABLED` defaults to `0`.
- `FALL_MONITORING_ANY_MOTION_THRESHOLD` defaults to `1843` in BMA423
  5.11-g units, approximately 0.90 g of slope.
- `FALL_MONITORING_ANY_MOTION_DURATION` defaults to `2` consecutive 50 Hz
  samples, or 40 ms.

These values are candidate-wake settings, not validated fall thresholds. The
low-g, impact, and post-impact stillness thresholds must be selected only after
the labeled dataset has been reviewed for false positives and missed events.

See [Power Optimization Results](power-optimization-results.md) for the wider
sensor and physical-measurement status.