# BCG Sample-Rate Study

The production WatchFace remains at an effective 25 Hz. No lower-rate policy
is enabled until labeled human traces show acceptable accuracy.

## Status

The capture protocol, shared C++ replay processor, synthetic regression tests,
and FIFO calculations are complete. Labeled human traces and simultaneous
reference BPM measurements are still pending, so 12.5 Hz remains an
experimental comparison only.

## Automatic Foreground Capture

Build the optional capture firmware with:

```powershell
pio run -e bcg-trace -t upload --upload-port COM3
```

On cold boot this profile automatically arms capture and opens Heart Rate.
Start the listener before resetting the installed firmware so it receives the
automatic export; use a timeout longer than the capture:

```powershell
python tools/capture_bcg_trace.py listen --port COM3 --timeout 120 `
  --output measurements/bcg/resting-72 --label resting --reference-bpm 72
```

The retained buffer holds `1500` XYZ samples. Foreground sampling currently
uses `samplePeriodMs = 40` and reports `25000` mHz, so filling the buffer takes
about `60 s` when samples are available. Startup calls `beginAutomatic(40000)`,
but the first appended sample replaces that rate with its actual reported
`25000` mHz. Always use the exported manifest's rate rather than inferring it
from the startup argument. Completion exports the trace and stops measurement.
Startup subsequently prints `@WATCHY_BCG_ERROR 1 measurement-ended`; a
successful listener has already returned on `@WATCHY_BCG_DONE`.

## Manual Commands

While Heart Rate is servicing serial input, the host supports `arm`, `status`,
`export`, and `clear` in addition to `listen`:

```powershell
python tools/capture_bcg_trace.py arm --port COM3
python tools/capture_bcg_trace.py status --port COM3
python tools/capture_bcg_trace.py export --port COM3 `
  --output measurements/bcg/resting-72 --label resting --reference-bpm 72
```

`arm` resets the buffer and disables automatic export. `export` can return a
partial capture; it is not proof of 60 seconds of data. Returning to a watch
face after arming is not a verified background-capture protocol: non-watch-face
sleep disables BCG, and USB runtime does not use the normal deep-sleep FIFO
wake path. Confirm active sensor ownership and collection before using that
workflow for a study.

`src/app/BcgProcessor.cpp` contains the processor used by both firmware and
the native replay tool. Build and replay require `g++` on PATH and are
performed automatically by:

```powershell
python tools/analyze_bcg_traces.py path/to/manifest.json --output report.json
```

## Dataset Format

Dataset manifests use this schema:

```json
{
  "schema_version": 1,
  "sample_rate_millihz": 25000,
  "traces": [
    {
      "file": "resting-72.csv",
      "label": "resting",
      "reference_bpm": 72,
      "movement_end_ms": null
    }
  ]
}
```

Each CSV must contain `x`, `y`, and `z` columns with signed raw BMA423
samples. The default comparison runs 25,000 mHz and 12,500 mHz. Candidate
rates must divide the source rate exactly so decimation is deterministic.

## Interpretation

The report includes valid-window percentage, mean and maximum BPM error,
detected/missed/extra beats, first-valid latency, optional post-movement
recovery time, and FIFO wake/overflow margins.

Required labels before changing production policy are: resting, slow heart
rate, normal heart rate, higher heart rate, mild movement, significant
movement, poor contact, and noisy readings. A simultaneous trusted reference
BPM is required for error claims. Synthetic traces verify the harness only;
they are not evidence that 12.5 Hz is acceptable.

With the current 170-sample capacity and 150-sample watermark:

| Effective rate | Wake interval | Overflow time | Margin | Wakes/hour |
| ---: | ---: | ---: | ---: | ---: |
| 25 Hz | 6.0 s | 6.8 s | 0.8 s | 600 |
| 12.5 Hz | 12.0 s | 13.6 s | 1.6 s | 300 |

The 20-sample headroom is preserved in both cases. These figures describe
wake opportunity only, not measured energy or BCG accuracy.

See [Power Optimization Results](power-optimization-results.md) for the
production policy and remaining physical measurement matrix.