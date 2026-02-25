#!/usr/bin/env python3
"""Arm, inspect, or export the optional RTC-retained Watchy BCG capture."""

from __future__ import annotations

import argparse
import csv
import json
import shutil
import tempfile
import time
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable

try:
    from tools.capture_gallery import ReconnectingSerial
except ModuleNotFoundError:
    from capture_gallery import ReconnectingSerial


PROTOCOL_VERSION = 1


class CaptureError(RuntimeError):
    """Raised when the BCG capture protocol or output is invalid."""


@dataclass(frozen=True)
class Sample:
    index: int
    x: int
    y: int
    z: int


@dataclass(frozen=True)
class Trace:
    sample_rate_millihz: int
    samples: tuple[Sample, ...]


def parse_export(lines: Iterable[str]) -> Trace:
    sample_rate = None
    expected_count = None
    samples: list[Sample] = []
    for raw_line in lines:
        fields = raw_line.strip().split()
        if not fields:
            continue
        if fields[0] == "@WATCHY_BCG_TRACE":
            if len(fields) != 4 or fields[1] != str(PROTOCOL_VERSION):
                raise CaptureError(f"invalid BCG trace header: {raw_line!r}")
            sample_rate = int(fields[2])
            expected_count = int(fields[3])
        elif fields[0] == "@WATCHY_BCG_SAMPLE":
            if sample_rate is None or len(fields) != 6:
                raise CaptureError(f"sample outside BCG trace: {raw_line!r}")
            sample = Sample(*(int(value) for value in fields[2:]))
            if fields[1] != str(PROTOCOL_VERSION) or sample.index != len(samples):
                raise CaptureError(f"BCG sample order mismatch: {raw_line!r}")
            samples.append(sample)
        elif fields[0] == "@WATCHY_BCG_DONE":
            if len(fields) != 3 or fields[1] != str(PROTOCOL_VERSION):
                raise CaptureError(f"invalid BCG completion: {raw_line!r}")
            completed_count = int(fields[2])
            if expected_count is None or completed_count != expected_count or \
                    len(samples) != expected_count:
                raise CaptureError(
                    f"BCG trace expected {expected_count} samples, "
                    f"received {len(samples)}"
                )
            return Trace(sample_rate, tuple(samples))
    raise CaptureError("BCG export ended before completion")


def write_dataset(trace: Trace, output: Path, label: str,
                  reference_bpm: float | None = None,
                  movement_end_ms: int | None = None,
                  replace: bool = False) -> None:
    output = output.resolve()
    output.parent.mkdir(parents=True, exist_ok=True)
    if output.exists():
        if not replace:
            raise CaptureError(f"output already exists (use --replace): {output}")
        if not output.is_dir():
            raise CaptureError(f"output is not a directory: {output}")
        shutil.rmtree(output)
    staging = Path(tempfile.mkdtemp(prefix=f".{output.name}-", dir=output.parent))
    try:
        filename = "trace.csv"
        with (staging / filename).open("w", encoding="ascii", newline="") as stream:
            writer = csv.writer(stream, lineterminator="\n")
            writer.writerow(("index", "x", "y", "z"))
            for sample in trace.samples:
                writer.writerow((sample.index, sample.x, sample.y, sample.z))
        entry: dict[str, object] = {"file": filename, "label": label}
        if reference_bpm is not None:
            entry["reference_bpm"] = reference_bpm
        if movement_end_ms is not None:
            entry["movement_end_ms"] = movement_end_ms
        manifest = {
            "schema_version": 1,
            "sample_rate_millihz": trace.sample_rate_millihz,
            "traces": [entry],
        }
        (staging / "manifest.json").write_text(
            json.dumps(manifest, indent=2, sort_keys=True) + "\n",
            encoding="ascii",
        )
        staging.rename(output)
    except Exception:
        shutil.rmtree(staging, ignore_errors=True)
        raise


def run_command(port: str, baud: int, timeout: float, action: str,
                serial_module=None) -> list[str]:
    if serial_module is None:
        try:
            import serial as serial_module
        except ImportError as error:
            raise CaptureError(
                "serial capture requires pyserial; "
                "install tools/requirements-gallery.txt"
            ) from error
    command = (
        None
        if action == "listen"
        else f"@WATCHY_BCG_{action.upper()} {PROTOCOL_VERSION}\n".encode("ascii")
    )
    command_retry_seconds = 0.5
    records: list[str] = []
    pending = bytearray()
    with ReconnectingSerial(
        serial_module, port, baud, min(timeout, 0.25), timeout
    ) as connection:
        deadline = time.monotonic() + timeout
        next_command_at = 0.0
        while time.monotonic() < deadline:
            now = time.monotonic()
            if command is not None and now >= next_command_at:
                connection.write(command)
                connection.flush()
                next_command_at = now + command_retry_seconds
            chunk = connection.read(256)
            if not chunk:
                continue
            pending.extend(chunk)
            while b"\n" in pending:
                raw, _, remainder = pending.partition(b"\n")
                pending = bytearray(remainder)
                line = raw.rstrip(b"\r").decode("ascii", errors="ignore")
                if not line.startswith("@WATCHY_BCG_"):
                    continue
                records.append(line)
                if line.startswith("@WATCHY_BCG_ERROR"):
                    raise CaptureError(line)
                if action != "listen":
                    command = None
                if action == "arm" and line.startswith("@WATCHY_BCG_ARMED"):
                    return records
                if action == "status" and line.startswith("@WATCHY_BCG_STATUS"):
                    return records
                if action == "clear" and line.startswith("@WATCHY_BCG_STATUS"):
                    return records
                if action in ("export", "listen") and line.startswith("@WATCHY_BCG_DONE"):
                    return records
    detail = f"; last record: {records[-1]}" if records else ""
    raise CaptureError(
        "timed out; install env bcg-trace and keep the Heart Rate app open"
        f"{detail}"
    )


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "action", choices=("arm", "status", "export", "clear", "listen")
    )
    parser.add_argument("--port", required=True)
    parser.add_argument("--baud", type=int, default=115200)
    parser.add_argument("--timeout", type=float, default=30.0)
    parser.add_argument("--output", type=Path)
    parser.add_argument("--label")
    parser.add_argument("--reference-bpm", type=float)
    parser.add_argument("--movement-end-ms", type=int)
    parser.add_argument("--replace", action="store_true")
    return parser


def main() -> int:
    args = build_parser().parse_args()
    records = run_command(args.port, args.baud, args.timeout, args.action)
    if args.action in ("export", "listen"):
        if args.output is None or not args.label:
            raise CaptureError("export requires --output and --label")
        trace = parse_export(records)
        write_dataset(
            trace, args.output, args.label, args.reference_bpm,
            args.movement_end_ms, args.replace,
        )
        print(f"Exported {len(trace.samples)} samples to {args.output}")
    else:
        print(records[-1])
    return 0


if __name__ == "__main__":
    raise SystemExit(main())