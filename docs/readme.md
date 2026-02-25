# Documentation

This directory separates implemented firmware contracts from studies that
still require physical Watchy measurements. Pending data is kept explicit so
proxy metrics are not presented as battery, accuracy, or timing results.

## Core Guides

- [Watchy Application SDK](watchy-sdk.md): lifecycle, UI, input, storage,
  hardware ownership, display refresh, and diagnostics contracts.
- [Deterministic Gallery](deterministic-gallery.md): isolated framebuffer
  capture, protocol, generated artifacts, and verification.
- [Power Optimization Results](power-optimization-results.md): current binary
  metrics, deterministic work reductions, and physical measurement backlog.
- [Metronome Timing](metronome-timing.md): cadence-worker design, host timing
  proof, diagnostics, and hardware jitter protocol.

## Calibration Studies

- [BCG Sample-Rate Study](bcg-rate-study.md): 25 Hz production policy,
  retained-trace capture, replay analysis, and 12.5 Hz candidate evaluation.
- [Step Counter ODR Study](step-odr-study.md): 50 Hz baseline and controlled
  25 Hz walking trial matrix.
- [Fall Monitoring Calibration](fall-monitoring.md): bounded candidate-event
  logging and labeled trace collection.

## Current Verification

Verification recorded on 2026-09-16 for the current working tree:

- Production and gallery builds pass. Production uses 83,988 B static RAM
  and 2,133,625 B Flash; gallery uses 84,092 B RAM and 2,137,989 B Flash.
- Captured all 348 scenes from the connected Watchy, reviewed contact sheets
  and corrected-screen details, and regenerated the gallery. All scenes are
  nonblank 200x200 images; all 696 PNG/PBM hashes match the manifest. All 16
  watchface images match the existing golden hashes, which were not changed.
  Normal firmware was restored after capture.
- The host runner reports `Ran 75 tests` and `FAILED (failures=1, skipped=2)`.
  The skips are the BCG replay class setup and the metronome timing test,
  both requiring a host C++ compiler. The failure is
  `test_release_build_enables_strict_warnings`: the current PlatformIO
  configuration lacks the expected `build_src_flags` entry.
- `python tools/sync_gallery_catalog.py --check` passes using
  [src/sdk/demo/GalleryAppCatalog.inc](../src/sdk/demo/GalleryAppCatalog.inc):
  142 apps, 348 scenes, matching scene IDs and README screenshot tables.
- The power-diagnostics build has not been revalidated during these fixes.
- Physical startup, panel output, button response, and the measurements marked
  pending in the studies remain unverified. Build success and source-contract
  tests are not substitutes for those checks.

### App Lifecycle Review

Static diagnostics (Chip Info, Heap Monitor, I2C Scanner, Wake Reason, Reset
Cause) now draw once and wait for BACK through `runStaticView()`. Screen Ruler
retains its BACK-only loop. Generators, converters, games, and astronomy views
retain their existing interactive loops. Sensor and Bluetooth helpers have
explicit release/stop paths; result-only OS/Bluetooth views may return while
retaining `APP_STATE`, with BACK handled by the core dispatcher.

NTP and Vibration Test results no longer automatically return to the menu.
Heart Rate startup failures now remain visible until BACK, except in automated
trace builds. Save/apply actions and explicit cancellations remain intentional
exits. This is a source-level lifecycle review and regression coverage, not a
claim that all 142 apps were operated with physical buttons or live services.

Hands-on acceptance remains: open each menu action, exercise its controls,
leave a static result idle, press BACK, and verify return navigation and radio,
sensor, and motor cleanup. Repeat applicable failure/cancellation paths and
USB/battery operation. Live Wi-Fi/BLE services and BCG accuracy were not tested
by the deterministic capture.