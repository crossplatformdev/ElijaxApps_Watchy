import binascii
import contextlib
import io
import json
import re
import tempfile
import unittest
from pathlib import Path

from tools import capture_gallery


def protocol_stream(scene_ids, bitmaps, *, width=8, height=1, corrupt_crc=False):
    stream = bytearray(b"ESP-ROM boot message\r\n@WATCHY_GALLERY 1\r\n")
    for sequence, (scene_id, bitmap) in enumerate(zip(scene_ids, bitmaps), 1):
        checksum = binascii.crc32(bitmap) & 0xFFFFFFFF
        if corrupt_crc and sequence == 1:
            checksum ^= 1
        stream.extend(
            f"@WATCHY_FRAME 1 {sequence} {width} {height} "
            f"{len(bitmap)} {checksum:08X} {scene_id}\n".encode()
        )
        stream.extend(bitmap)
        stream.extend(f"\n@WATCHY_END {sequence}\n".encode())
    stream.extend(f"@WATCHY_DONE {len(scene_ids)} {len(scene_ids)}\n".encode())
    return bytes(stream)


class CaptureGalleryTests(unittest.TestCase):
    def parse(self, payload, scene_ids):
        reader = capture_gallery.TimedReader(io.BytesIO(payload), False, 1.0)
        return capture_gallery.parse_capture(reader, scene_ids, 8, 1)

    def test_valid_stream_writes_inverted_pbm_png_and_manifest(self):
        scene_ids = ("demo/black/light", "demo/white/light")
        frames = self.parse(protocol_stream(scene_ids, (b"\x00", b"\xFF")), scene_ids)
        with tempfile.TemporaryDirectory() as temporary:
            output = Path(temporary) / "gallery"
            capture_gallery.write_gallery(frames, output)
            pbm = (output / "pbm/demo/black/light.pbm").read_bytes()
            png = (output / "png/demo/black/light.png").read_bytes()
            manifest = json.loads((output / "manifest.json").read_text())
            self.assertTrue(pbm.endswith(b"\xFF"))
            self.assertTrue(png.startswith(b"\x89PNG\r\n\x1a\n"))
            self.assertEqual(manifest["frame_count"], 2)
            self.assertEqual(manifest["frames"][1]["id"], scene_ids[1])

    def test_corrupt_frame_is_rejected(self):
        with self.assertRaisesRegex(capture_gallery.CaptureError, "CRC32 mismatch"):
            self.parse(protocol_stream(("demo/frame/light",), (b"\xAA",), corrupt_crc=True),
                       ("demo/frame/light",))

    def test_unexpected_scene_is_rejected(self):
        with self.assertRaisesRegex(capture_gallery.CaptureError, "scene 1 mismatch"):
            self.parse(protocol_stream(("demo/wrong/light",), (b"\xAA",)),
                       ("demo/expected/light",))

    def test_serial_handshake_requests_capture(self):
        class Duplex(io.BytesIO):
            def __init__(self):
                super().__init__(b"@WATCHY_READY 1\r\n")
                self.sent = bytearray()

            def write(self, value):
                self.sent.extend(value)
                return len(value)

            def flush(self):
                pass

        stream = Duplex()
        reader = capture_gallery.TimedReader(stream, False, 1.0)
        self.assertFalse(capture_gallery.request_serial_capture(reader, stream))
        self.assertEqual(stream.sent, b"@WATCHY_CAPTURE 1\n")

    def test_catalog_matches_firmware_emission_order(self):
        repository = Path(__file__).resolve().parents[1]
        source = (repository / "src/demo/DeterministicGallery.cpp").read_text()
        array_order = (
            "menuSceneIds",
            "clockSceneIds",
            "timeToolSceneIds",
            "sensorSceneIds",
            "utilitySceneIds",
            "gameSceneIds",
            "healthcareSceneIds",
            "systemSceneIds",
            "osUtilitySceneIds",
            "astronomySceneIds",
            "networkSceneIds",
            "bluetoothSceneIds",
        )
        firmware_scene_ids = []
        for array_name in array_order:
            match = re.search(
                rf"const char \*const {array_name}\[\] = \{{(.*?)\}};",
                source,
                re.DOTALL,
            )
            self.assertIsNotNone(match, f"missing firmware array {array_name}")
            firmware_scene_ids.extend(re.findall(r'"([^"]+)"', match.group(1)))
        catalog = capture_gallery.load_catalog(repository / "tools/gallery_scene_ids.txt")
        self.assertEqual(len(catalog), 153)
        self.assertEqual(tuple(firmware_scene_ids), catalog)

    def test_full_catalog_replay_publishes_153_scenes(self):
        repository = Path(__file__).resolve().parents[1]
        catalog_path = repository / "tools/gallery_scene_ids.txt"
        catalog = capture_gallery.load_catalog(catalog_path)
        bitmaps = tuple(bytes([sequence % 256]) * 5000 for sequence in range(153))
        payload = protocol_stream(catalog, bitmaps, width=200, height=200)
        with tempfile.TemporaryDirectory() as temporary:
            input_path = Path(temporary) / "capture.bin"
            output_path = Path(temporary) / "gallery"
            input_path.write_bytes(payload)
            with contextlib.redirect_stdout(io.StringIO()):
                result = capture_gallery.main(
                    (
                        "--input",
                        str(input_path),
                        "--catalog",
                        str(catalog_path),
                        "--output",
                        str(output_path),
                    )
                )
            manifest = json.loads((output_path / "manifest.json").read_text())
            self.assertEqual(result, 0)
            self.assertEqual(manifest["frame_count"], 153)
            self.assertEqual(len(tuple((output_path / "png").rglob("*.png"))), 153)
            self.assertEqual(len(tuple((output_path / "pbm").rglob("*.pbm"))), 153)

    def test_readme_publishes_every_captured_scene_once(self):
        repository = Path(__file__).resolve().parents[1]
        gallery = repository / "docs/gallery"
        manifest = json.loads((gallery / "manifest.json").read_text())
        readme = (repository / "README.md").read_text()
        expected_paths = tuple(
            f"docs/gallery/{frame['png']}" for frame in manifest["frames"]
        )
        published_paths = re.findall(
            r'<img src="(docs/gallery/png/[^"]+\.png)"', readme
        )
        application_rows = re.findall(
            r"^\| \*\*(?!Total\*\*)[^|]+\*\* \|", readme, re.MULTILINE
        )

        self.assertEqual(manifest["frame_count"], 153)
        self.assertCountEqual(published_paths, expected_paths)
        self.assertEqual(len(set(published_paths)), 153)
        self.assertEqual(len(application_rows), 142)
        for relative_path in expected_paths:
            self.assertTrue((repository / relative_path).is_file(), relative_path)


if __name__ == "__main__":
    unittest.main()