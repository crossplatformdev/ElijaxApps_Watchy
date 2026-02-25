#!/usr/bin/env python3
"""Export bounded Watchy fall-calibration traces over USB serial."""

from __future__ import annotations

import argparse
import csv
import json
import math
import shutil
import sys
import tempfile
import time
from dataclasses import asdict, dataclass
from pathlib import Path
from typing import Iterable, Sequence, TextIO


PROTOCOL_VERSION = 1
ACCEL_LSB_PER_G = 256.0


class TraceError(RuntimeError):
    """Raised when a fall-trace export violates its protocol."""


@dataclass(frozen=True)
class Sample:
    index: int
    phase: str
    relative_ms: int
    x: int
    y: int
    z: int

    @property
    def magnitude_g(self) -> float:
        return math.sqrt(self.x * self.x + self.y * self.y + self.z * self.z) / ACCEL_LSB_PER_G


@dataclass(frozen=True)
class Trace:
    sequence: int
    captured_at: int
    sample_rate_hz: int
    pre_count: int
    post_count: int
    interrupt_status: str
    samples: tuple[Sample, ...]


def parse_export(lines: Iterable[str]) -> tuple[Trace, ...]:
    traces: list[Trace] = []
    header: tuple[int, int, int, int, int, str] | None = None
    samples: list[Sample] = []

    for raw_line in lines:
        line = raw_line.strip()
        if not line:
            continue
        fields = line.split()
        record = fields[0]
        if record == "@WATCHY_FALL_TRACE":
            if header is not None or len(fields) != 8:
                raise TraceError(f"invalid trace header: {line!r}")
            try:
                version = int(fields[1])
                header = (
                    int(fields[2]), int(fields[3]), int(fields[4]),
                    int(fields[5]), int(fields[6]), fields[7],
                )
            except ValueError as error:
                raise TraceError(f"invalid trace header: {line!r}") from error
            if version != PROTOCOL_VERSION:
                raise TraceError(f"unsupported trace protocol: {version}")
            samples = []
        elif record == "@WATCHY_FALL_SAMPLE":
            if header is None or len(fields) != 8:
                raise TraceError(f"sample outside a trace: {line!r}")
            try:
                sequence = int(fields[1])
                sample = Sample(
                    int(fields[2]), fields[3], int(fields[4]),
                    int(fields[5]), int(fields[6]), int(fields[7]),
                )
            except ValueError as error:
                raise TraceError(f"invalid sample: {line!r}") from error
            if sequence != header[0] or sample.index != len(samples):
                raise TraceError(f"sample order mismatch: {line!r}")
            if sample.phase not in ("P", "A"):
                raise TraceError(f"invalid sample phase: {line!r}")
            samples.append(sample)
        elif record == "@WATCHY_FALL_END":
            if header is None or len(fields) != 2:
                raise TraceError(f"invalid trace end: {line!r}")
            sequence = int(fields[1])
            expected_count = header[3] + header[4]
            if sequence != header[0] or len(samples) != expected_count:
                raise TraceError(
                    f"trace {header[0]} expected {expected_count} samples, "
                    f"received {len(samples)}"
                )
            traces.append(Trace(*header, tuple(samples)))
            header = None
            samples = []
        elif record == "@WATCHY_FALL_ERROR":
            raise TraceError(f"device reported an invalid stored trace: {line}")
        elif record == "@WATCHY_FALL_DONE":
            if header is not None or len(fields) != 2:
                raise TraceError(f"invalid completion record: {line!r}")
            expected_traces = int(fields[1])
            if len(traces) != expected_traces:
                raise TraceError(
                    f"device reported {expected_traces} traces, received {len(traces)}"
                )
            return tuple(traces)

    raise TraceError("trace export ended before completion")


def write_export(traces: Sequence[Trace], output: Path, label: str,
                 replace: bool = False) -> None:
    output = output.resolve()
    output.parent.mkdir(parents=True, exist_ok=True)
    if output.exists():
        if not replace:
            raise TraceError(f"output already exists (use --replace): {output}")
        if not output.is_dir():
            raise TraceError(f"output is not a directory: {output}")
        shutil.rmtree(output)

    staging = Path(tempfile.mkdtemp(prefix=f".{output.name}-", dir=output.parent))
    try:
        manifest_traces = []
        for trace in traces:
            filename = f"trace-{trace.sequence:04d}.csv"
            with (staging / filename).open("w", encoding="ascii", newline="") as stream:
                writer = csv.writer(stream, lineterminator="\n")
                writer.writerow(
                    ("index", "phase", "relative_ms", "x", "y", "z", "magnitude_g")
                )
                for sample in trace.samples:
                    writer.writerow(
                        (
                            sample.index, sample.phase, sample.relative_ms,
                            sample.x, sample.y, sample.z,
                            f"{sample.magnitude_g:.6f}",
                        )
                    )
            magnitudes = tuple(sample.magnitude_g for sample in trace.samples)
            metadata = asdict(trace)
            metadata.pop("samples")
            metadata.update(
                {
                    "file": filename,
                    "sample_count": len(trace.samples),
                    "minimum_magnitude_g": min(magnitudes) if magnitudes else None,
                    "maximum_magnitude_g": max(magnitudes) if magnitudes else None,
                }
            )
            manifest_traces.append(metadata)

        manifest = {
            "schema_version": 1,
            "protocol_version": PROTOCOL_VERSION,
            "label": label,
            "accelerometer_range_g": 8,
            "trace_count": len(traces),
            "traces": manifest_traces,
        }
        (staging / "manifest.json").write_text(
            json.dumps(manifest, indent=2, sort_keys=True) + "\n",
            encoding="ascii",
        )
        staging.rename(output)
    except Exception:
        shutil.rmtree(staging, ignore_errors=True)
        raise


def read_serial_export(port: str, baud: int, timeout: float) -> tuple[Trace, ...]:
    try:
        import serial
    except ImportError as error:
        raise TraceError(
            "serial export requires pyserial; install tools/requirements-gallery.txt"
        ) from error

    deadline = time.monotonic() + timeout
    records: list[str] = []
    with serial.Serial(port, baud, timeout=min(timeout, 0.25)) as connection:
        connection.reset_input_buffer()
        connection.write(b"@WATCHY_FALL_EXPORT 1\n")
        connection.flush()
        while time.monotonic() < deadline:
            raw = connection.readline()
            if not raw:
                continue
            try:
                line = raw.decode("ascii").strip()
            except UnicodeDecodeError:
                continue
            if not line.startswith("@WATCHY_FALL_"):
                continue
            records.append(line)
            if line.startswith("@WATCHY_FALL_DONE"):
                return parse_export(records)
    raise TraceError("timed out waiting for Fall Monitor export; open the app first")


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--port", required=True, help="Watchy serial port, for example COM3")
    parser.add_argument("--label", required=True, help="activity label, for example running")
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--baud", type=int, default=115200)
    parser.add_argument("--timeout", type=float, default=20.0)
    parser.add_argument("--replace", action="store_true")
    return parser


def main(argv: Sequence[str] | None = None) -> int:
    args = build_parser().parse_args(argv)
    try:
        if args.timeout <= 0:
            raise TraceError("timeout must be greater than zero")
        traces = read_serial_export(args.port, args.baud, args.timeout)
        write_export(traces, args.output, args.label, args.replace)
        print(f"Exported {len(traces)} fall traces to {args.output.resolve()}")
        return 0
    except (OSError, TraceError) as error:
        print(f"capture_fall_traces: error: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())