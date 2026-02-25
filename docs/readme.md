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

As of 2026-08-26, the production, gallery, and power-diagnostics firmware
builds pass. The host suite reports 68 tests, and the synchronized gallery
catalog contains 142 applications and 348 scenes. Physical measurements marked
pending in the linked studies have not been inferred from those checks.