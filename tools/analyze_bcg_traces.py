#!/usr/bin/env python3
"""Compare BCG trace datasets with the firmware's shared C++ processor."""

from __future__ import annotations

import argparse
import json
import statistics
import subprocess
import tempfile
from dataclasses import dataclass
from pathlib import Path
from typing import Sequence


BASELINE_RATE_MILLIHZ = 25000
DEFAULT_RATES_MILLIHZ = (25000, 12500)
FIFO_CAPACITY_SAMPLES = 170
FIFO_WATERMARK_SAMPLES = 150


class AnalysisError(RuntimeError):
    """Raised when a BCG dataset or replay output is invalid."""


@dataclass(frozen=True)
class Window:
    rate_millihz: int
    processed_samples: int
    valid: bool
    bpm: int
    cumulative_beats: int


@dataclass(frozen=True)
class Replay:
    rate_millihz: int
    input_samples: int
    processed_samples: int
    windows: tuple[Window, ...]
    valid_windows: int
    detected_beats: int
    first_valid_ms: int


@dataclass(frozen=True)
class TraceDefinition:
    path: Path
    label: str
    sample_rate_millihz: int
    reference_bpm: float | None
    movement_end_ms: int | None


def compile_replay(repository: Path, output: Path) -> None:
    subprocess.run(
        [
            "g++", "-std=c++17", "-O2", "-Wall", "-Wextra", "-Werror",
            "-Isrc", "tools/bcg_replay.cpp", "src/app/BcgProcessor.cpp",
            "-o", str(output),
        ],
        cwd=repository,
        check=True,
    )


def parse_replay(output: str) -> Replay:
    windows: list[Window] = []
    summary = None
    for line in output.splitlines():
        fields = line.split()
        if not fields:
            continue
        if fields[0] == "@WATCHY_BCG_WINDOW" and len(fields) == 7:
            if fields[1] != "1":
                raise AnalysisError("unsupported BCG window protocol")
            windows.append(
                Window(
                    rate_millihz=int(fields[2]),
                    processed_samples=int(fields[3]),
                    valid=fields[4] == "1",
                    bpm=int(fields[5]),
                    cumulative_beats=int(fields[6]),
                )
            )
        elif fields[0] == "@WATCHY_BCG_SUMMARY" and len(fields) == 9:
            if fields[1] != "1":
                raise AnalysisError("unsupported BCG summary protocol")
            summary = tuple(int(value) for value in fields[2:])
    if summary is None:
        raise AnalysisError("BCG replay did not emit a summary")
    rate, input_count, processed, window_count, valid, beats, first_ms = summary
    if window_count != len(windows) or valid != sum(window.valid for window in windows):
        raise AnalysisError("BCG replay summary does not match its windows")
    if any(window.rate_millihz != rate for window in windows):
        raise AnalysisError("BCG replay mixed sample rates")
    return Replay(rate, input_count, processed, tuple(windows), valid, beats, first_ms)


def load_dataset(manifest_path: Path) -> tuple[TraceDefinition, ...]:
    try:
        manifest = json.loads(manifest_path.read_text(encoding="ascii"))
    except (OSError, json.JSONDecodeError) as error:
        raise AnalysisError(f"cannot read {manifest_path}: {error}") from error
    if manifest.get("schema_version") != 1:
        raise AnalysisError(f"unsupported dataset schema in {manifest_path}")
    sample_rate = int(manifest.get("sample_rate_millihz", 0))
    traces = []
    for entry in manifest.get("traces", ()):
        trace_path = manifest_path.parent / entry["file"]
        traces.append(
            TraceDefinition(
                path=trace_path,
                label=str(entry["label"]),
                sample_rate_millihz=sample_rate,
                reference_bpm=(
                    float(entry["reference_bpm"])
                    if entry.get("reference_bpm") is not None
                    else None
                ),
                movement_end_ms=(
                    int(entry["movement_end_ms"])
                    if entry.get("movement_end_ms") is not None
                    else None
                ),
            )
        )
    if not traces:
        raise AnalysisError(f"dataset contains no traces: {manifest_path}")
    return tuple(traces)


def run_replay(executable: Path, trace: TraceDefinition, rate: int) -> Replay:
    result = subprocess.run(
        [str(executable), str(trace.path), str(trace.sample_rate_millihz), str(rate)],
        check=True,
        capture_output=True,
        text=True,
    )
    return parse_replay(result.stdout)


def fifo_metrics(rate_millihz: int) -> dict[str, float]:
    samples_per_second = rate_millihz / 1000.0
    return {
        "sample_rate_hz": samples_per_second,
        "fifo_capacity_samples": FIFO_CAPACITY_SAMPLES,
        "watermark_samples": FIFO_WATERMARK_SAMPLES,
        "wake_interval_seconds": FIFO_WATERMARK_SAMPLES / samples_per_second,
        "overflow_seconds": FIFO_CAPACITY_SAMPLES / samples_per_second,
        "service_margin_seconds": (
            FIFO_CAPACITY_SAMPLES - FIFO_WATERMARK_SAMPLES
        ) / samples_per_second,
        "wakes_per_hour": 3600.0 * samples_per_second / FIFO_WATERMARK_SAMPLES,
    }


def trace_metrics(trace: TraceDefinition, replay: Replay) -> dict[str, object]:
    duration_seconds = replay.processed_samples * 1000.0 / replay.rate_millihz
    valid_bpms = [window.bpm for window in replay.windows if window.valid]
    metrics: dict[str, object] = {
        "label": trace.label,
        "rate_millihz": replay.rate_millihz,
        "duration_seconds": duration_seconds,
        "window_count": len(replay.windows),
        "valid_result_percentage": (
            replay.valid_windows * 100.0 / len(replay.windows)
            if replay.windows else 0.0
        ),
        "detected_beats": replay.detected_beats,
        "first_valid_ms": replay.first_valid_ms,
    }
    if trace.reference_bpm is not None:
        errors = [abs(bpm - trace.reference_bpm) for bpm in valid_bpms]
        expected_beats = round(trace.reference_bpm * duration_seconds / 60.0)
        metrics.update(
            {
                "reference_bpm": trace.reference_bpm,
                "mean_bpm_error": statistics.fmean(errors) if errors else None,
                "maximum_bpm_error": max(errors) if errors else None,
                "missed_beats": max(0, expected_beats - replay.detected_beats),
                "extra_beats": max(0, replay.detected_beats - expected_beats),
            }
        )
    if trace.movement_end_ms is not None:
        recovery = None
        for window in replay.windows:
            window_ms = window.processed_samples * 1000000 // replay.rate_millihz
            if window.valid and window_ms >= trace.movement_end_ms:
                recovery = window_ms - trace.movement_end_ms
                break
        metrics["movement_recovery_ms"] = recovery
    return metrics


def aggregate(metrics: Sequence[dict[str, object]], rate: int) -> dict[str, object]:
    selected = [metric for metric in metrics if metric["rate_millihz"] == rate]
    errors = [
        float(metric["mean_bpm_error"])
        for metric in selected
        if metric.get("mean_bpm_error") is not None
    ]
    maxima = [
        float(metric["maximum_bpm_error"])
        for metric in selected
        if metric.get("maximum_bpm_error") is not None
    ]
    return {
        "rate_millihz": rate,
        "trace_count": len(selected),
        "mean_valid_result_percentage": statistics.fmean(
            float(metric["valid_result_percentage"]) for metric in selected
        ),
        "mean_bpm_error": statistics.fmean(errors) if errors else None,
        "maximum_bpm_error": max(maxima) if maxima else None,
        "missed_beats": sum(int(metric.get("missed_beats", 0)) for metric in selected),
        "extra_beats": sum(int(metric.get("extra_beats", 0)) for metric in selected),
        "fifo": fifo_metrics(rate),
    }


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("manifests", nargs="+", type=Path)
    parser.add_argument(
        "--rates", type=int, nargs="+", default=DEFAULT_RATES_MILLIHZ,
        help="candidate rates in millihertz",
    )
    parser.add_argument("--output", type=Path)
    return parser


def main() -> int:
    args = build_parser().parse_args()
    repository = Path(__file__).resolve().parents[1]
    traces = tuple(
        trace
        for manifest in args.manifests
        for trace in load_dataset(manifest)
    )
    with tempfile.TemporaryDirectory() as temporary:
        executable = Path(temporary) / "bcg_replay.exe"
        compile_replay(repository, executable)
        metrics = [
            trace_metrics(trace, run_replay(executable, trace, rate))
            for trace in traces
            for rate in args.rates
        ]
    report = {
        "schema_version": 1,
        "processor": "src/app/BcgProcessor.cpp",
        "rates": [aggregate(metrics, rate) for rate in args.rates],
        "traces": metrics,
        "conclusion": (
            "candidate comparison available"
            if all(trace.reference_bpm is not None for trace in traces)
            else "insufficient evidence: reference BPM missing"
        ),
    }
    payload = json.dumps(report, indent=2, sort_keys=True) + "\n"
    if args.output:
        args.output.write_text(payload, encoding="ascii", newline="\n")
    else:
        print(payload, end="")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())