# Deterministic Gallery

The gallery build captures the framebuffer produced by the firmware's real
renderers. It covers one to five distinct views for all 142 registered
application actions plus the root menu, ten category menus, nine SDK
calibration scenes, and light/dark baselines for eight WatchFaces, for 348
scenes in total. The default firmware and its display behavior are unchanged.

## Status

The checked-in catalog currently contains 142 applications and 348 ordered
scenes. `python tools/sync_gallery_catalog.py --check` is the authoritative
consistency check for the catalog, scene IDs, and root readme screenshot rows.

The output is suitable for a public demo:

- Time is fixed at `2026-08-23 10:34:45`.
- The theme is fixed to Light without reading Preferences.
- Location, health, network, Bluetooth, sensor, and random values are fixed,
  fictitious fixtures.
- Network examples use documentation domains and address blocks. Bluetooth
  examples use locally administered addresses.
- The gallery entry point bypasses `Watchy::init()`. It does not initialize
  Wi-Fi, BLE, sensors, vibration, tasks, NVS, RTC, or sleep.
- E-paper image writes, refreshes, power-off, and hibernation are no-ops in the
  gallery build. Capture occurs at the owned display-driver boundary.

## Capture

Install the host dependency once:

```powershell
python -m pip install -r tools/requirements-gallery.txt
```

Build and upload the isolated firmware profile, replacing `COM7` with the
Watchy's port:

```powershell
python tools/sync_gallery_catalog.py
pio run -e gallery
pio run -e gallery -t upload --upload-port COM7
```

Start the receiver. The firmware waits for its explicit request, so there is
no timing window after upload:

```powershell
python tools/capture_gallery.py --port COM7 --output docs/gallery
```

The receiver refuses to overwrite an existing directory. Use `--replace` for
an intentional repeat capture:

```powershell
python tools/capture_gallery.py --port COM7 --output docs/gallery --replace
```

No reset is required between captures. The gallery firmware returns to the
ready state and resets its frame sequence for every host request.

## Output

The receiver commits the output directory only after the complete stream has
passed validation. A successful run creates:

```text
docs/gallery/
  manifest.json
  pbm/<category>/<application>/<state>/light.pbm
  png/<category>/<application>/<state>/light.png
```

PNG files are lossless, native-size, 1-bit grayscale images. PBM files use the
binary `P4` format; their bits are inverted because PBM represents black as
`1`, while the GxEPD framebuffer represents white as `1`.

`manifest.json` preserves protocol order and records each scene ID, dimensions,
raw framebuffer CRC32, output paths, and SHA-256 hashes. It contains no capture
timestamp, serial port, device identifier, or personal data, so identical
framebuffers produce identical artifacts and manifests.

Application prefixes and their one-to-five state slugs live in
`src/demo/GalleryAppCatalog.inc`. `tools/sync_gallery_catalog.py` derives the
ordered `tools/gallery_scene_ids.txt` protocol contract and the five README
screenshot columns from that source. The receiver fails unless all 348
expected IDs arrive exactly once and in order, each at 200x200 with 5,000
bytes and a valid CRC32, followed by matching end and completion records.

## Protocol

The transport is USB CDC at 115200 baud. Records are ASCII lines except for
the fixed-size raw framebuffer payload:

```text
Device: @WATCHY_READY 1
Host:   @WATCHY_CAPTURE 1
Device: @WATCHY_GALLERY 1
Device: @WATCHY_FRAME 1 <sequence> <width> <height> <bytes> <crc32> <scene-id>
Device: <raw framebuffer bytes>
Device: @WATCHY_END <sequence>
Device: @WATCHY_DONE <actual-count> <expected-count>
```

Bootloader text before the ready or gallery banner is ignored. Any malformed
record, unknown scene, duplicate, gap, dimension mismatch, byte-count mismatch,
checksum failure, firmware error, or incomplete run prevents publication of
the output directory.

## Verification

The host parser and encoders require no board for testing:

```powershell
python tools/sync_gallery_catalog.py --check
python -m unittest tools.test_capture_gallery
```

The tests exercise the handshake, framing, CRC rejection, exact catalog order,
PBM inversion, PNG creation, manifest generation, README publication, and
checked-in asset coverage. They do not open or render the generated images.
Run the complete host suite with
`python -m unittest discover -s tools -p "test_*.py"` before publishing a
capture.