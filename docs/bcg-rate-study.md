# BCG Sample-Rate Study

The production WatchFace remains at an effective 25 Hz. No lower-rate policy
is enabled until labeled human traces show acceptable accuracy.

## Status

The capture protocol, shared C++ replay processor, synthetic regression tests,
and FIFO calculations are complete. Labeled human traces and simultaneous
reference BPM measurements are still pending, so 12.5 Hz remains an
experimental comparison only.

## Capture

Build the optional capture firmware with:

```powershell
pio run -e bcg-trace
```

After installing that build, open Heart Rate and arm a capture while the USB
serial connection is available:

```powershell
python tools/capture_bcg_trace.py arm --port COM3
```

Return to the 7-SEG WatchFace and keep the requested condition for 60 seconds.
Then open Heart Rate again and export the retained samples:

```powershell
python tools/capture_bcg_trace.py export --port COM3 `
  --output measurements/bcg/resting-72 --label resting --reference-bpm 72
```

`src/app/BcgProcessor.cpp` contains the processor used by both firmware and
the native replay tool. Build and replay are performed automatically by:

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