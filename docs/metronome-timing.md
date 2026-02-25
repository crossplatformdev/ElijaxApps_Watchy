# Metronome Timing

The musical Metronome and CPR Metronome share one cadence worker. The worker
owns the vibration pin while active; application code owns controls and
rendering. E-paper work therefore cannot postpone scheduling until the UI loop
returns.

## Status

Deadline arithmetic and worker ownership are covered by deterministic host
tests. Production, gallery, and diagnostics firmware builds pass. Physical
GPIO onset, motor response, and long-run on-watch jitter remain pending.

## Runtime Contract

- `esp_timer_get_time()` supplies the monotonic microsecond clock.
- The first deadline is armed before the running screen is rendered.
- Each next deadline advances from the previous deadline, never from the time
  at which a delayed task happens to resume.
- Fractional microseconds are retained, so integer division cannot accumulate
  long-run tempo drift.
- After a late wake, obsolete beat slots are skipped and at most one pulse is
  emitted. The worker never produces a catch-up burst.
- Musical beats use a 20 ms envelope and a 40 ms accent on the configured beat
  boundary. CPR uses a uniform 30 ms envelope at 110 BPM.
- BACK and pause request a bounded worker stop and force the motor pin low.
- The 2,048-byte task stack exists only while a cadence app is running.

Metronome BPM and accent settings use the checksummed `WatchySdk::Storage`
record path. They survive normal deep sleep, resets, and power loss. Invalid or
incompatible records fall back to 100 BPM with an accent every four beats.

## Deterministic Verification

Run the native timing probe with the complete host suite:

```powershell
python -m unittest discover -s tools -p "test_*.py"
```

`tools/metronome_timing_probe.cpp` verifies every integer tempo from 30 through
240 BPM for 12 simulated hours. It requires exact deadline phase, correct
accent placement, and one-beat behavior after a simulated multi-period stall.
The current result covers 211 tempos with `phase_error_us=0`, `skip=ok`, and
`accent=ok`.

This proves the deadline arithmetic, not physical motor timing. FreeRTOS wake
latency, GPIO onset, and the motor's mechanical response require the Watchy.

## On-Device Validation

The `power-diagnostics` environment records these fields without printing on
each beat or writing telemetry to NVS/Flash:

- `metronome_beats`: pulses started by the worker;
- `metronome_skipped`: obsolete beat slots skipped after a late wake;
- `metronome_max_late_us`: maximum GPIO scheduling lateness against the
  absolute deadline;
- `metronome_stack_free_words`: minimum unused worker stack words.

Build and install the diagnostics firmware. For each test tempo, start the
collector before waking the watch, run the Metronome for at least ten minutes,
exit the app, and wake once more so the completed counters are emitted. For a
120 BPM session:

```powershell
pio run -e power-diagnostics
python tools/capture_power_diagnostics.py --port COM3 --duration 900 `
  --output measurements/power/metronome-120.json
```

Repeat with separate output files at 30 and 240 BPM. Include the running-screen
dirty update and repeated pause, resume, BACK, and re-entry.
The firmware acceptance target is zero skipped beats and no worker-stop
timeouts. Record `metronome_max_late_us`; a preliminary GPIO-onset target is
at most 5,000 us while the display updates. Confirm pulse intervals and drift
with a logic analyzer on `VIB_MOTOR_PIN`. A current probe or accelerometer on
the case is still required to characterize mechanical vibration onset.

Physical timing results remain pending until that protocol is run on hardware.

See [Power Optimization Results](power-optimization-results.md) for current
binary metrics and the complete physical-measurement backlog.