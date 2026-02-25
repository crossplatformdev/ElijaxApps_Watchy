# Contributing

This repository builds an application-focused firmware on top of SQFMI
Watchy. Search the
[project issues](https://github.com/crossplatformdev/ElijaxApps_Watchy/issues)
and
[pull requests](https://github.com/crossplatformdev/ElijaxApps_Watchy/pulls)
before starting overlapping work. Upstream hardware or base-library problems
may belong in the [SQFMI Watchy tracker](https://github.com/sqfmi/Watchy/issues).

## Development workflow

1. Fork the repository and create a focused branch from `main`.
2. Keep changes compatible with Watchy v3 / ESP32-S3 and the existing
	 `WatchyUi`, sensor ownership, display, storage, and power contracts.
3. Run the required checks:

```powershell
python -m unittest discover -s tools -p "test_*.py"
python tools/sync_gallery_catalog.py --check
pio run -e esp32-s3-devkitc-1
```

4. If renderers changed, build and inspect the deterministic gallery. Do not
	 approve WatchFace pixel changes by regenerating golden hashes without an
	 explicit visual review.
5. Open a pull request that explains user-visible behavior, validation, and
	 any hardware measurements still pending.

## Project rules

- Keep network, health, location, and identity fixtures fictitious in public
	gallery artifacts.
- Stop Wi-Fi, BLE, sensor foreground modes, worker tasks, and vibration on all
	application exit paths.
- Prefer focused tests and deterministic gallery states for error, empty,
	loading, and success behavior.
- Do not claim battery, accuracy, or timing improvements from proxy metrics
	alone.

For Watchy community discussion, use the
[SQFMI Discord](https://discord.gg/ZXDegGV8E7).
