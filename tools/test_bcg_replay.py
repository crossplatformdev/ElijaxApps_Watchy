import subprocess
import shutil
import tempfile
import unittest
from pathlib import Path


class BcgReplayTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.repository = Path(__file__).resolve().parents[1]
        cls.compiler = next(
            (shutil.which(name) for name in ("g++", "clang++", "c++") if shutil.which(name)),
            None,
        )
        if cls.compiler is None:
            raise unittest.SkipTest(
                "a host C++ compiler (g++, clang++, or c++) is required"
            )
        cls.temporary = tempfile.TemporaryDirectory()
        cls.executable = Path(cls.temporary.name) / "bcg_replay.exe"
        subprocess.run(
            [
                cls.compiler,
                "-std=c++17",
                "-O2",
                "-Wall",
                "-Wextra",
                "-Werror",
                "-Isrc",
                "tools/bcg_replay.cpp",
                "src/app/BcgProcessor.cpp",
                "-o",
                str(cls.executable),
            ],
            cwd=cls.repository,
            check=True,
            capture_output=True,
            text=True,
        )

    @classmethod
    def tearDownClass(cls):
        cls.temporary.cleanup()

    def make_trace(self, sample_rate_hz=25, seconds=90, bpm=60):
        path = Path(self.temporary.name) / f"synthetic-{bpm}.csv"
        beat_interval = sample_rate_hz * 60 // bpm
        pulse_shape = (40, 80, 140, 240, 180, 100, 40)
        with path.open("w", encoding="ascii", newline="\n") as stream:
            stream.write("index,x,y,z\n")
            for index in range(sample_rate_hz * seconds):
                phase = index % beat_interval
                pulse = pulse_shape[phase] if phase < len(pulse_shape) else 0
                stream.write(f"{index},0,0,{1024 + pulse}\n")
        return path

    def replay(self, trace, target_rate_millihz):
        result = subprocess.run(
            [str(self.executable), str(trace), "25000", str(target_rate_millihz)],
            check=True,
            capture_output=True,
            text=True,
        )
        lines = result.stdout.splitlines()
        summary = next(line for line in lines if line.startswith("@WATCHY_BCG_SUMMARY"))
        fields = summary.split()
        return {
            "rate": int(fields[2]),
            "windows": int(fields[5]),
            "valid_windows": int(fields[6]),
            "beats": int(fields[7]),
            "first_valid_ms": int(fields[8]),
        }

    def test_same_processor_replays_25_and_12_5_hz(self):
        trace = self.make_trace()
        baseline = self.replay(trace, 25000)
        candidate = self.replay(trace, 12500)
        self.assertEqual(baseline["windows"], 6)
        self.assertEqual(candidate["windows"], 6)
        self.assertGreater(baseline["valid_windows"], 0)
        self.assertGreater(candidate["valid_windows"], 0)
        self.assertGreater(baseline["beats"], 80)
        self.assertGreater(candidate["beats"], 80)

    def test_rejects_non_divisor_rate(self):
        trace = self.make_trace(seconds=1)
        result = subprocess.run(
            [str(self.executable), str(trace), "25000", "12000"],
            capture_output=True,
            text=True,
        )
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("integer divisor", result.stderr)


if __name__ == "__main__":
    unittest.main()