# 7-SEG WatchFace Example

This directory preserves the standalone Arduino example and its local fonts,
icons, settings, and renderer. It is separate from the integrated application
suite implementation in
[`src/watchfaces/7_SEG`](../../../src/watchfaces/7_SEG/).

Set `DARKMODE` near the top of `Watchy_7_SEG.cpp` to `true` for a dark face or
`false` for a light face, then rebuild the sketch. Configure weather and Wi-Fi
values in `settings.h` before installing it.

The integrated firmware selects light or dark appearance through Watchy OS and
protects both polarities with deterministic pixel baselines. Changes made only
to this standalone example do not alter the integrated WatchFace.
