import unittest

from tools import capture_power_diagnostics


class PowerDiagnosticsTests(unittest.TestCase):
    def test_parses_scalar_and_phase_lists(self):
        record = capture_power_diagnostics.parse_record(
            "@WATCHY_POWER wakes=4 dirty_pixels=1512 "
            "net_heap=220000,180000,160000,150000,218000 "
            "net_block=110000,90000,70000,65000,108000"
        )
        self.assertEqual(record["wakes"], 4)
        self.assertEqual(record["dirty_pixels"], 1512)
        self.assertEqual(record["net_heap"][2], 160000)
        self.assertEqual(record["net_block"][-1], 108000)

    def test_rejects_non_numeric_value(self):
        with self.assertRaisesRegex(
            capture_power_diagnostics.DiagnosticsError, "invalid.*value"
        ):
            capture_power_diagnostics.parse_record(
                "@WATCHY_POWER wakes=unknown"
            )


if __name__ == "__main__":
    unittest.main()