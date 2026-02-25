# Step Counter ODR Study

Production remains at the existing 50 Hz baseline. The included Bosch driver
documents the available ODR values and wrist step-counter parameters, but does
not state that 25 Hz preserves counting accuracy. The current Bosch product
site no longer exposes a retrievable BMA423 product datasheet, so no lower ODR
is enabled from assumption alone.

## Status

The isolated 25 Hz candidate build and trial protocol are ready. Physical
walking trials have not been recorded, so production remains at 50 Hz.

The `step-odr-25` PlatformIO environment changes only the baseline
accelerometer ODR:

```powershell
pio run -e step-odr-25
```

Use a non-7-SEG WatchFace and disable Fall Monitor during each trial so BCG or
fall acquisition does not request a higher-power sensor mode. For both the
production 50 Hz build and the 25 Hz candidate:

1. Install the build and note the initial Step Counter value.
2. Walk exactly 100 counted steps at a slow pace.
3. Record the counter delta.
4. Complete three trials at each slow, normal, and fast walking pace.
5. Record battery voltage, surface, footwear, and whether the watch was worn
   on the dominant wrist.

| ODR | Pace | Reference/trial | Trial 1 delta | Trial 2 delta | Trial 3 delta | Mean absolute error |
| ---: | --- | ---: | ---: | ---: | ---: | ---: |
| 50 Hz | slow | 100 | pending | pending | pending | pending |
| 50 Hz | normal | 100 | pending | pending | pending | pending |
| 50 Hz | fast | 100 | pending | pending | pending | pending |
| 25 Hz | slow | 100 | pending | pending | pending | pending |
| 25 Hz | normal | 100 | pending | pending | pending | pending |
| 25 Hz | fast | 100 | pending | pending | pending | pending |

Do not promote 25 Hz unless repeated trials show no meaningful deterioration
in slow, normal, or fast walking. This study measures counting accuracy; power
savings still require current or long-duration battery measurements.

See [Power Optimization Results](power-optimization-results.md) for the
production sensor policy and measurement backlog.