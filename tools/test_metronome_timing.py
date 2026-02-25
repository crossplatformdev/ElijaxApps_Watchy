import subprocess
import shutil
import tempfile
import unittest
from pathlib import Path


class MetronomeTimingTests(unittest.TestCase):
    def test_full_bpm_range_has_no_schedule_drift_or_catch_up_burst(self):
        repository = Path(__file__).resolve().parents[1]
        compiler = next(
            (shutil.which(name) for name in ("g++", "clang++", "c++") if shutil.which(name)),
            None,
        )
        if compiler is None:
            self.skipTest("a host C++ compiler (g++, clang++, or c++) is required")
        with tempfile.TemporaryDirectory() as temporary:
            executable = Path(temporary) / "metronome_timing_probe.exe"
            subprocess.run(
                [
                    compiler,
                    "-std=c++17",
                    "-O2",
                    "-Wall",
                    "-Wextra",
                    "-Werror",
                    "-Isrc",
                    "tools/metronome_timing_probe.cpp",
                    "-o",
                    str(executable),
                ],
                cwd=repository,
                check=True,
                capture_output=True,
                text=True,
            )
            result = subprocess.run(
                [str(executable)],
                check=True,
                capture_output=True,
                text=True,
            )

        self.assertIn("bpms=211", result.stdout)
        self.assertIn("hours=12", result.stdout)
        self.assertIn("phase_error_us=0", result.stdout)
        self.assertIn("skip=ok", result.stdout)
        self.assertIn("accent=ok", result.stdout)


if __name__ == "__main__":
    unittest.main()