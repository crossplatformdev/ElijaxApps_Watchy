import json
import tempfile
import unittest
from pathlib import Path

from tools import capture_fall_traces


class FallTraceTests(unittest.TestCase):
    def test_parse_and_write_trace(self):
        records = (
            "boot message",
            "@WATCHY_FALL_TRACE 1 7 1787500000 25 2 1 0040",
            "@WATCHY_FALL_SAMPLE 7 0 P -80 0 0 256",
            "@WATCHY_FALL_SAMPLE 7 1 P -40 0 0 128",
            "@WATCHY_FALL_SAMPLE 7 2 A 0 0 0 768",
            "@WATCHY_FALL_END 7",
            "@WATCHY_FALL_DONE 1",
        )
        traces = capture_fall_traces.parse_export(records)
        self.assertEqual(len(traces), 1)
        self.assertEqual(traces[0].sequence, 7)
        self.assertAlmostEqual(traces[0].samples[0].magnitude_g, 1.0)

        with tempfile.TemporaryDirectory() as temporary:
            output = Path(temporary) / "running"
            capture_fall_traces.write_export(traces, output, "running")
            manifest = json.loads((output / "manifest.json").read_text())
            self.assertEqual(manifest["label"], "running")
            self.assertEqual(manifest["trace_count"], 1)
            self.assertEqual(manifest["traces"][0]["maximum_magnitude_g"], 3.0)
            self.assertTrue((output / "trace-0007.csv").is_file())

    def test_rejects_incomplete_trace(self):
        records = (
            "@WATCHY_FALL_TRACE 1 1 0 25 1 1 0040",
            "@WATCHY_FALL_SAMPLE 1 0 P -40 0 0 256",
            "@WATCHY_FALL_END 1",
        )
        with self.assertRaisesRegex(capture_fall_traces.TraceError, "expected 2 samples"):
            capture_fall_traces.parse_export(records)


if __name__ == "__main__":
    unittest.main()