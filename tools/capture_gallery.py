#!/usr/bin/env python3
"""Receive and validate the deterministic Watchy framebuffer gallery."""

from __future__ import annotations

import argparse
import binascii
import hashlib
import json
import re
import shutil
import struct
import sys
import tempfile
import time
import zlib
from dataclasses import dataclass
from pathlib import Path
from typing import BinaryIO, Sequence


PROTOCOL_VERSION = 1
DEFAULT_WIDTH = 200
DEFAULT_HEIGHT = 200
SCENE_COMPONENT = re.compile(r"^[A-Za-z0-9][A-Za-z0-9._-]*$")


class CaptureError(RuntimeError):
    """Raised when a gallery stream violates the capture contract."""


@dataclass(frozen=True)
class Frame:
    sequence: int
    scene_id: str
    width: int
    height: int
    crc32: int
    bitmap: bytes


class TimedReader:
    def __init__(self, stream: BinaryIO, live: bool, idle_timeout: float):
        self.stream = stream
        self.live = live
        self.idle_timeout = idle_timeout

    def read_exact(self, size: int, context: str) -> bytes:
        data = bytearray()
        deadline = time.monotonic() + self.idle_timeout
        while len(data) < size:
            chunk = self.stream.read(size - len(data))
            if chunk:
                data.extend(chunk)
                deadline = time.monotonic() + self.idle_timeout
                continue
            if not self.live:
                raise CaptureError(f"unexpected end of input while reading {context}")
            if time.monotonic() >= deadline:
                raise CaptureError(f"timed out while reading {context}")
        return bytes(data)

    def read_line(self, context: str, maximum: int = 4096) -> bytes:
        data = bytearray()
        while len(data) < maximum:
            value = self.read_exact(1, context)
            data.extend(value)
            if value == b"\n":
                return bytes(data)
        raise CaptureError(f"{context} exceeds {maximum} bytes")


def decode_line(raw: bytes, context: str, tolerant: bool = False) -> str:
    try:
        return raw.rstrip(b"\r\n").decode("ascii")
    except UnicodeDecodeError as error:
        if tolerant:
            return raw.rstrip(b"\r\n").decode("ascii", errors="replace")
        raise CaptureError(f"non-ASCII data in {context}") from error


def next_nonempty_line(reader: TimedReader, context: str) -> str:
    while True:
        line = decode_line(reader.read_line(context), context)
        if line:
            return line


def validate_scene_id(scene_id: str) -> tuple[str, ...]:
    components = tuple(scene_id.split("/"))
    if not components or any(
        component in ("", ".", "..") or not SCENE_COMPONENT.fullmatch(component)
        for component in components
    ):
        raise CaptureError(f"unsafe scene ID: {scene_id!r}")
    return components


def load_catalog(path: Path) -> tuple[str, ...]:
    try:
        lines = path.read_text(encoding="ascii").splitlines()
    except OSError as error:
        raise CaptureError(f"cannot read scene catalog {path}: {error}") from error
    scene_ids = tuple(line.strip() for line in lines if line.strip() and not line.startswith("#"))
    if not scene_ids:
        raise CaptureError(f"scene catalog is empty: {path}")
    for scene_id in scene_ids:
        validate_scene_id(scene_id)
    if len(set(scene_ids)) != len(scene_ids):
        raise CaptureError(f"scene catalog contains duplicate IDs: {path}")
    return scene_ids


def parse_frame_header(line: str) -> tuple[int, int, int, int, int, str]:
    fields = line.split(" ", 7)
    if len(fields) != 8 or fields[0] != "@WATCHY_FRAME":
        raise CaptureError(f"invalid frame header: {line!r}")
    try:
        version = int(fields[1])
        sequence = int(fields[2])
        width = int(fields[3])
        height = int(fields[4])
        byte_count = int(fields[5])
        checksum = int(fields[6], 16)
    except ValueError as error:
        raise CaptureError(f"invalid numeric field in frame header: {line!r}") from error
    if version != PROTOCOL_VERSION:
        raise CaptureError(f"unsupported frame protocol version: {version}")
    if not re.fullmatch(r"[0-9A-Fa-f]{8}", fields[6]):
        raise CaptureError(f"invalid CRC32 in frame header: {fields[6]!r}")
    validate_scene_id(fields[7])
    return sequence, width, height, byte_count, checksum, fields[7]


def parse_capture(
    reader: TimedReader,
    expected_scene_ids: Sequence[str],
    expected_width: int = DEFAULT_WIDTH,
    expected_height: int = DEFAULT_HEIGHT,
    banner_received: bool = False,
) -> list[Frame]:
    if not banner_received:
        while True:
            line = decode_line(reader.read_line("gallery banner"), "gallery banner", tolerant=True)
            if line == f"@WATCHY_GALLERY {PROTOCOL_VERSION}":
                break
            if line.startswith("@WATCHY_ERROR"):
                raise CaptureError(f"device reported an error before the banner: {line}")
            if line.startswith("@WATCHY_DONE"):
                raise CaptureError("gallery completion arrived before its protocol banner")

    frames: list[Frame] = []
    seen_scene_ids: set[str] = set()
    while True:
        line = next_nonempty_line(reader, "protocol record")
        if line.startswith("@WATCHY_ERROR"):
            raise CaptureError(f"device reported an error: {line}")
        if line.startswith("@WATCHY_DONE"):
            fields = line.split()
            if len(fields) != 3:
                raise CaptureError(f"invalid completion record: {line!r}")
            try:
                actual_count = int(fields[1])
                device_expected_count = int(fields[2])
            except ValueError as error:
                raise CaptureError(f"invalid completion record: {line!r}") from error
            catalog_count = len(expected_scene_ids)
            if actual_count != len(frames):
                raise CaptureError(
                    f"device reported {actual_count} frames, received {len(frames)}"
                )
            if device_expected_count != catalog_count:
                raise CaptureError(
                    f"device expects {device_expected_count} frames, catalog expects {catalog_count}"
                )
            if len(frames) != catalog_count:
                raise CaptureError(f"received {len(frames)} of {catalog_count} frames")
            return frames
        if not line.startswith("@WATCHY_FRAME"):
            raise CaptureError(f"unexpected protocol record: {line!r}")

        sequence, width, height, byte_count, expected_crc, scene_id = parse_frame_header(line)
        expected_sequence = len(frames) + 1
        if sequence != expected_sequence:
            raise CaptureError(
                f"frame sequence mismatch: expected {expected_sequence}, received {sequence}"
            )
        if sequence > len(expected_scene_ids):
            raise CaptureError(f"received unexpected extra frame {sequence}: {scene_id}")
        catalog_scene_id = expected_scene_ids[sequence - 1]
        if scene_id != catalog_scene_id:
            raise CaptureError(
                f"scene {sequence} mismatch: expected {catalog_scene_id!r}, received {scene_id!r}"
            )
        if scene_id in seen_scene_ids:
            raise CaptureError(f"duplicate scene ID: {scene_id}")
        if width != expected_width or height != expected_height:
            raise CaptureError(
                f"{scene_id}: expected {expected_width}x{expected_height}, received {width}x{height}"
            )
        expected_bytes = (width * height + 7) // 8
        if byte_count != expected_bytes:
            raise CaptureError(
                f"{scene_id}: expected {expected_bytes} bytes, header declares {byte_count}"
            )

        bitmap = reader.read_exact(byte_count, f"frame {sequence} bitmap")
        actual_crc = binascii.crc32(bitmap) & 0xFFFFFFFF
        if actual_crc != expected_crc:
            raise CaptureError(
                f"{scene_id}: CRC32 mismatch, expected {expected_crc:08X}, got {actual_crc:08X}"
            )
        if reader.read_exact(1, f"frame {sequence} delimiter") != b"\n":
            raise CaptureError(f"frame {sequence} has no newline before its end marker")
        end_line = decode_line(
            reader.read_line(f"frame {sequence} end marker"),
            f"frame {sequence} end marker",
        )
        if end_line != f"@WATCHY_END {sequence}":
            raise CaptureError(f"invalid frame end marker: {end_line!r}")

        frames.append(Frame(sequence, scene_id, width, height, actual_crc, bitmap))
        seen_scene_ids.add(scene_id)


def request_serial_capture(reader: TimedReader, stream: BinaryIO) -> bool:
    """Wait for a ready record and request a capture.

    Returns true when connected to an older sender that already emitted its
    gallery banner, allowing the parser to continue without another banner.
    """
    while True:
        line = decode_line(reader.read_line("gallery readiness"), "gallery readiness", tolerant=True)
        if line == f"@WATCHY_READY {PROTOCOL_VERSION}":
            stream.write(f"@WATCHY_CAPTURE {PROTOCOL_VERSION}\n".encode("ascii"))
            stream.flush()
            return False
        if line == f"@WATCHY_GALLERY {PROTOCOL_VERSION}":
            return True
        if line.startswith("@WATCHY_ERROR"):
            raise CaptureError(f"device reported an error while waiting: {line}")


def png_chunk(chunk_type: bytes, payload: bytes) -> bytes:
    checksum = binascii.crc32(chunk_type + payload) & 0xFFFFFFFF
    return struct.pack(">I", len(payload)) + chunk_type + payload + struct.pack(">I", checksum)


def encode_png(frame: Frame) -> bytes:
    if frame.width % 8 != 0:
        raise CaptureError("PNG encoding requires a byte-aligned framebuffer width")
    row_bytes = frame.width // 8
    scanlines = b"".join(
        b"\x00" + frame.bitmap[offset : offset + row_bytes]
        for offset in range(0, len(frame.bitmap), row_bytes)
    )
    header = struct.pack(">IIBBBBB", frame.width, frame.height, 1, 0, 0, 0, 0)
    return (
        b"\x89PNG\r\n\x1a\n"
        + png_chunk(b"IHDR", header)
        + png_chunk(b"IDAT", zlib.compress(scanlines, level=9))
        + png_chunk(b"IEND", b"")
    )


def encode_pbm(frame: Frame) -> bytes:
    header = f"P4\n{frame.width} {frame.height}\n".encode("ascii")
    return header + bytes(value ^ 0xFF for value in frame.bitmap)


def sha256(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def write_gallery(frames: Sequence[Frame], output: Path, replace: bool = False) -> None:
    if not frames:
        raise CaptureError("cannot write an empty gallery")
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
        manifest_frames = []
        for frame in frames:
            components = validate_scene_id(frame.scene_id)
            relative_base = Path(*components)
            pbm_relative = Path("pbm") / relative_base.with_suffix(".pbm")
            png_relative = Path("png") / relative_base.with_suffix(".png")
            pbm = encode_pbm(frame)
            png = encode_png(frame)
            for relative, content in ((pbm_relative, pbm), (png_relative, png)):
                destination = staging / relative
                destination.parent.mkdir(parents=True, exist_ok=True)
                destination.write_bytes(content)
            manifest_frames.append(
                {
                    "sequence": frame.sequence,
                    "id": frame.scene_id,
                    "width": frame.width,
                    "height": frame.height,
                    "byte_count": len(frame.bitmap),
                    "framebuffer_crc32": f"{frame.crc32:08X}",
                    "pbm": pbm_relative.as_posix(),
                    "pbm_sha256": sha256(pbm),
                    "png": png_relative.as_posix(),
                    "png_sha256": sha256(png),
                }
            )
        manifest = {
            "schema_version": 1,
            "protocol_version": PROTOCOL_VERSION,
            "frame_count": len(frames),
            "display": {
                "width": frames[0].width,
                "height": frames[0].height,
                "format": "1-bit row-major MSB-first; 0=black, 1=white",
            },
            "frames": manifest_frames,
        }
        (staging / "manifest.json").write_text(
            json.dumps(manifest, indent=2, sort_keys=True) + "\n",
            encoding="ascii",
            newline="\n",
        )
        staging.rename(output)
    except Exception:
        shutil.rmtree(staging, ignore_errors=True)
        raise


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    source = parser.add_mutually_exclusive_group(required=True)
    source.add_argument("--port", help="USB serial port, for example COM7")
    source.add_argument("--input", type=Path, help="previously recorded binary protocol stream")
    parser.add_argument("--baud", type=int, default=115200, help="serial baud rate")
    parser.add_argument("--timeout", type=float, default=30.0, help="maximum idle seconds")
    parser.add_argument(
        "--catalog",
        type=Path,
        default=Path(__file__).with_name("gallery_scene_ids.txt"),
        help="ordered expected scene ID catalog",
    )
    parser.add_argument("--output", type=Path, default=Path("gallery-output"))
    parser.add_argument("--replace", action="store_true", help="replace an existing output directory")
    return parser


def main(argv: Sequence[str] | None = None) -> int:
    args = build_parser().parse_args(argv)
    try:
        expected_scene_ids = load_catalog(args.catalog)
        if len(expected_scene_ids) != 153:
            raise CaptureError(
                f"scene catalog must contain 153 IDs, found {len(expected_scene_ids)}"
            )
        if args.timeout <= 0:
            raise CaptureError("timeout must be greater than zero")

        if args.input is not None:
            with args.input.open("rb") as stream:
                frames = parse_capture(TimedReader(stream, False, args.timeout), expected_scene_ids)
        else:
            try:
                import serial
            except ImportError as error:
                raise CaptureError(
                    "serial capture requires pyserial; install tools/requirements-gallery.txt"
                ) from error
            with serial.Serial(args.port, args.baud, timeout=min(args.timeout, 0.25)) as stream:
                reader = TimedReader(stream, True, args.timeout)
                banner_received = request_serial_capture(reader, stream)
                frames = parse_capture(
                    reader, expected_scene_ids, banner_received=banner_received
                )

        write_gallery(frames, args.output, args.replace)
        print(f"Captured {len(frames)} validated scenes in {args.output.resolve()}")
        return 0
    except (CaptureError, OSError) as error:
        print(f"capture_gallery: error: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())