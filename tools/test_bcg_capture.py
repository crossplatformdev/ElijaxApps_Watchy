import json
import tempfile
import time
import unittest
from pathlib import Path

from tools import capture_bcg_trace


class BcgCaptureTests(unittest.TestCase):
    def test_trace_build_keeps_usb_awake_during_foreground_capture(self):
        repository = Path(__file__).resolve().parents[1]
        ui_source = (repository / "src/sdk/WatchyUi.cpp").read_text()
        scheduled_wait = ui_source.split("Event Input::waitScheduled", 1)[1].split(
            "Event Input::waitNotified", 1
        )[0]
        self.assertIn("waitForButtonNotification(remaining)", scheduled_wait)
        self.assertNotIn("Power::idle", scheduled_wait)
        source = (repository / "src/app/ShowHeartRate.cpp").read_text()
        self.assertIn("WatchyUi::Input::waitScheduled(serialServiceIntervalMs)", source)
        watchy = (repository / "src/sdk/WatchyCore.cpp").read_text()
        init_start = watchy.split("void beginBootRuntime()", 1)[1].split(
            "void normalizeMenuStateAfterBoot", 1
        )[0]
        self.assertIn("setCpuFrequencyMhz(240);", init_start)
        self.assertIn("Serial.begin(115200);", init_start)
        self.assertIn("while (!Serial", init_start)
        self.assertIn("@WATCHY_BCG_READY", init_start)
        reset_path = watchy.split("void handleTraceCaptureBoot()", 1)[1].split(
            "void handleColdBoot", 1
        )[0]
        self.assertIn("#ifdef WATCHY_BCG_TRACE_CAPTURE", reset_path)
        self.assertIn("showHeartRate();", reset_path)
        self.assertIn("WatchyBcgTrace::beginAutomatic(40000);", reset_path)
        self.assertIn("@WATCHY_BCG_ERROR 1 measurement-ended", reset_path)
        self.assertIn("WatchyBcgTrace::serviceSerial();", reset_path)
        self.assertIn("traceSerialGracePeriodMs = 5000", reset_path)
        self.assertNotIn("while (true)", reset_path)
        heart_rate = (repository / "src/app/ShowHeartRate.cpp").read_text()
        self.assertIn("WatchyBcgTrace::measurementComplete()", heart_rate)

    def test_parse_and_write_dataset(self):
        trace = capture_bcg_trace.parse_export(
            (
                "@WATCHY_BCG_TRACE 1 25000 2",
                "@WATCHY_BCG_SAMPLE 1 0 -4 8 1024",
                "@WATCHY_BCG_SAMPLE 1 1 -3 7 1026",
                "@WATCHY_BCG_DONE 1 2",
            )
        )
        self.assertEqual(trace.sample_rate_millihz, 25000)
        self.assertEqual(trace.samples[1].z, 1026)
        with tempfile.TemporaryDirectory() as temporary:
            output = Path(temporary) / "resting"
            capture_bcg_trace.write_dataset(
                trace, output, "resting", reference_bpm=72
            )
            manifest = json.loads((output / "manifest.json").read_text())
            self.assertEqual(manifest["traces"][0]["reference_bpm"], 72)
            self.assertTrue((output / "trace.csv").is_file())

    def test_rejects_missing_sample(self):
        with self.assertRaisesRegex(capture_bcg_trace.CaptureError, "expected 2"):
            capture_bcg_trace.parse_export(
                (
                    "@WATCHY_BCG_TRACE 1 25000 2",
                    "@WATCHY_BCG_SAMPLE 1 0 0 0 1024",
                    "@WATCHY_BCG_DONE 1 2",
                )
            )

    def test_foreground_rate_is_preserved(self):
        trace = capture_bcg_trace.parse_export(
            (
                "@WATCHY_BCG_TRACE 1 40000 1",
                "@WATCHY_BCG_SAMPLE 1 0 1 2 3",
                "@WATCHY_BCG_DONE 1 1",
            )
        )
        self.assertEqual(trace.sample_rate_millihz, 40000)

    def test_command_retries_until_usb_receiver_is_ready(self):
        class Stream:
            def __init__(self, timeout):
                self.timeout = timeout
                self.writes = 0
                self.response = bytearray()

            def write(self, value):
                self.writes += 1
                if self.writes == 2:
                    self.response.extend(b"@WATCHY_BCG_STATUS 1 0 1500 1 0\n")
                return len(value)

            def flush(self):
                pass

            def read(self, size):
                if not self.response:
                    time.sleep(self.timeout)
                    return b""
                value = bytes(self.response[:size])
                del self.response[:size]
                return value

            def close(self):
                pass

        class SerialModule:
            SerialException = OSError

            def __init__(self):
                self.stream = None

            def Serial(self, port, baud, timeout):
                self.stream = Stream(timeout)
                return self.stream

        serial_module = SerialModule()
        records = capture_bcg_trace.run_command(
            "COM3", 115200, 2.0, "status", serial_module
        )
        self.assertEqual(records[-1], "@WATCHY_BCG_STATUS 1 0 1500 1 0")
        self.assertEqual(serial_module.stream.writes, 2)


if __name__ == "__main__":
    unittest.main()