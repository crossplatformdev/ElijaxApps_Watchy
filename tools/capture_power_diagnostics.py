#!/usr/bin/env python3
"""Capture RTC-retained @WATCHY_POWER records from a reconnecting serial port."""

from __future__ import annotations

import argparse
import json
import time
from pathlib import Path
from typing import Iterable

try:
    from tools.capture_gallery import ReconnectingSerial
except ModuleNotFoundError:
    from capture_gallery import ReconnectingSerial


class DiagnosticsError(RuntimeError):
    """Raised when power diagnostics cannot be parsed or captured."""


def parse_record(line: str) -> dict[str, int | list[int]]:
    fields = line.strip().split()
    if not fields or fields[0] != "@WATCHY_POWER":
        raise DiagnosticsError(f"not a WATCHY_POWER record: {line!r}")
    record: dict[str, int | list[int]] = {}
    for field in fields[1:]:
        if "=" not in field:
            raise DiagnosticsError(f"invalid WATCHY_POWER field: {field!r}")
        key, raw_value = field.split("=", 1)
        try:
            record[key] = (
                [int(value) for value in raw_value.split(",")]
                if "," in raw_value
                else int(raw_value)
            )
        except ValueError as error:
            raise DiagnosticsError(
                f"invalid WATCHY_POWER value: {field!r}"
            ) from error
    return record


def parse_records(lines: Iterable[str]) -> list[dict[str, int | list[int]]]:
    return [parse_record(line) for line in lines if line.startswith("@WATCHY_POWER")]


def capture(port: str, baud: int, duration: float,
            reconnect_timeout: float) -> list[dict[str, int | list[int]]]:
    try:
        import serial
    except ImportError as error:
        raise DiagnosticsError(
            "serial capture requires pyserial; install tools/requirements-gallery.txt"
        ) from error
    deadline = time.monotonic() + duration
    lines: list[str] = []
    pending = bytearray()
    with ReconnectingSerial(
        serial, port, baud, min(0.25, duration), reconnect_timeout
    ) as stream:
        while time.monotonic() < deadline:
            chunk = stream.read(256)
            if not chunk:
                continue
            pending.extend(chunk)
            while b"\n" in pending:
                raw_line, _, remainder = pending.partition(b"\n")
                pending = bytearray(remainder)
                line = raw_line.rstrip(b"\r").decode("ascii", errors="ignore")
                if line.startswith("@WATCHY_POWER"):
                    lines.append(line)
    return parse_records(lines)


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--port", required=True)
    parser.add_argument("--baud", type=int, default=115200)
    parser.add_argument("--duration", type=float, default=90.0)
    parser.add_argument("--reconnect-timeout", type=float, default=30.0)
    parser.add_argument("--minimum-records", type=int, default=1)
    parser.add_argument("--output", type=Path)
    return parser


def main() -> int:
    args = build_parser().parse_args()
    if args.duration <= 0 or args.reconnect_timeout <= 0:
        raise DiagnosticsError("durations must be greater than zero")
    records = capture(
        args.port, args.baud, args.duration, args.reconnect_timeout
    )
    if len(records) < args.minimum_records:
        raise DiagnosticsError(
            f"expected at least {args.minimum_records} records, got {len(records)}"
        )
    payload = json.dumps(
        {"schema_version": 1, "records": records}, indent=2
    ) + "\n"
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(payload, encoding="ascii", newline="\n")
    else:
        print(payload, end="")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())