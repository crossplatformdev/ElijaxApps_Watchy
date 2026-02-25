import unittest

from tools import analyze_bcg_traces


class BcgAnalysisTests(unittest.TestCase):
    def test_parse_replay_and_fifo_math(self):
        replay = analyze_bcg_traces.parse_replay(
            "\n".join(
                (
                    "@WATCHY_BCG_WINDOW 1 12500 188 1 72 18",
                    "@WATCHY_BCG_SUMMARY 1 12500 376 188 1 1 18 15040",
                )
            )
        )
        self.assertEqual(replay.rate_millihz, 12500)
        self.assertEqual(replay.windows[0].bpm, 72)
        fifo = analyze_bcg_traces.fifo_metrics(12500)
        self.assertEqual(fifo["wake_interval_seconds"], 12.0)
        self.assertEqual(fifo["service_margin_seconds"], 1.6)
        self.assertEqual(fifo["wakes_per_hour"], 300.0)

    def test_rejects_inconsistent_summary(self):
        with self.assertRaisesRegex(
            analyze_bcg_traces.AnalysisError, "does not match"
        ):
            analyze_bcg_traces.parse_replay(
                "@WATCHY_BCG_SUMMARY 1 25000 375 375 1 1 10 15000"
            )


if __name__ == "__main__":
    unittest.main()