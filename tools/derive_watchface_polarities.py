#!/usr/bin/env python3
"""Infer authored polarity for each registered watchface.

We define "authored polarity" as the polarity the watchface code/assets were
originally written for:
- BlackOnWhite: black ink on white paper (common Watchy faces)
- WhiteOnBlack: white ink on black paper

This script scans watchface source trees for strong signals:
- display.fillScreen(GxEPD_BLACK) vs display.fillScreen(GxEPD_WHITE)
- drawBitmap(..., GxEPD_WHITE) vs drawBitmap(..., GxEPD_BLACK)
- setTextColor(GxEPD_WHITE) vs setTextColor(GxEPD_BLACK)

It uses the registered ordering in src/sdk/WatchfaceRegistry.cpp (kWatchfaces)
so the output can be copied directly into UiSDK::polarityForFaceId().

Usage:
  python tools/derive_watchface_polarities.py

Outputs:
- tools/watchface_polarities_report.md
- prints a C++ switch snippet for UiSDK.cpp
"""

from __future__ import annotations

import re
from dataclasses import dataclass
from pathlib import Path
from typing import Dict, Iterable, List, Tuple

REPO_ROOT = Path(__file__).resolve().parents[1]
REGISTRY_CPP = REPO_ROOT / "src" / "sdk" / "WatchfaceRegistry.cpp"
WATCHFACES_ROOT = REPO_ROOT / "src" / "watchfaces"
OS_WRAPPERS_ROOT = REPO_ROOT / "src" / "os"
REPORT_MD = REPO_ROOT / "tools" / "watchface_polarities_report.md"

SOURCE_EXTS = {".c", ".cc", ".cpp", ".cxx", ".h", ".hpp", ".ino"}


@dataclass(frozen=True)
class FaceSignals:
    fill_black: int = 0
    fill_white: int = 0
    bitmap_black: int = 0
    bitmap_white: int = 0
    text_black: int = 0
    text_white: int = 0
    rect_black: int = 0
    rect_white: int = 0
    sdk_theme_refs: int = 0


def _read_text(path: Path) -> str:
    try:
        return path.read_text(encoding="utf-8", errors="replace")
    except Exception:
        return ""


def parse_registry_order(registry_cpp: Path) -> List[str]:
    text = _read_text(registry_cpp)

    # Extract the kWatchfaces initializer list entries: {"Name", &...}
    entries = re.findall(r"\{\s*\"([^\"]+)\"\s*,\s*&", text)
    if not entries:
        raise RuntimeError("Failed to parse watchface list from WatchfaceRegistry.cpp")

    # The first 77 of these correspond to kWatchfaces. There may be other lists;
    # confirm by also reading kWatchfaceCount.
    m = re.search(r"const\s+uint8_t\s+kWatchfaceCount\s*=\s*(\d+)\s*;", text)
    if not m:
        raise RuntimeError("Failed to parse kWatchfaceCount")
    count = int(m.group(1))

    if len(entries) < count:
        raise RuntimeError(f"Registry parsed {len(entries)} names but expected {count}")

    return entries[:count]


def iter_sources_in_dir(root: Path) -> Iterable[Path]:
    for p in root.rglob("*"):
        if not p.is_file():
            continue
        if p.suffix.lower() in SOURCE_EXTS:
            yield p


def iter_sources_for_face(face_name: str) -> Iterable[Path]:
    face_dir = WATCHFACES_ROOT / face_name
    if face_dir.exists():
        yield from iter_sources_in_dir(face_dir)

    # Also scan the OS wrapper, since some watchfaces are implemented purely
    # as wrappers using UiSDK (e.g., Basic).
    wrapper_candidates = [
        OS_WRAPPERS_ROOT / f"WatchfaceDraw{face_name}.cpp",
        OS_WRAPPERS_ROOT / f"WatchfaceDraw{face_name.replace('-', '')}.cpp",
    ]
    for p in wrapper_candidates:
        if p.exists() and p.is_file():
            yield p


def scan_signals(face_name: str) -> FaceSignals:
    fill_black = fill_white = 0
    bitmap_black = bitmap_white = 0
    text_black = text_white = 0
    rect_black = rect_white = 0
    sdk_theme_refs = 0

    # Simple regexes; keep them permissive to handle whitespace/namespace.
    re_fill_black = re.compile(r"fillScreen\s*\(\s*(?:GxEPD_)?BLACK\s*\)")
    re_fill_white = re.compile(r"fillScreen\s*\(\s*(?:GxEPD_)?WHITE\s*\)")

    re_bitmap_black = re.compile(r"drawBitmap\s*\([^;\n]*?(?:GxEPD_)?BLACK\s*\)")
    re_bitmap_white = re.compile(r"drawBitmap\s*\([^;\n]*?(?:GxEPD_)?WHITE\s*\)")

    re_text_black = re.compile(r"setTextColor\s*\(\s*(?:GxEPD_)?BLACK\s*(?:,|\))")
    re_text_white = re.compile(r"setTextColor\s*\(\s*(?:GxEPD_)?WHITE\s*(?:,|\))")

    re_rect_black = re.compile(r"fillRect\s*\([^;\n]*?(?:GxEPD_)?BLACK\s*\)")
    re_rect_white = re.compile(r"fillRect\s*\([^;\n]*?(?:GxEPD_)?WHITE\s*\)")

    re_sdk_theme = re.compile(r"\bUiSDK::(getWatchfaceBg|getWatchfaceFg|renderApp)\b|\bBASE_POLARITY\b")

    for src in iter_sources_for_face(face_name):
        t = _read_text(src)
        if not t:
            continue

        fill_black += len(re_fill_black.findall(t))
        fill_white += len(re_fill_white.findall(t))

        # Count separately even if both match the same line; this is signal only.
        bitmap_black += len(re_bitmap_black.findall(t))
        bitmap_white += len(re_bitmap_white.findall(t))

        text_black += len(re_text_black.findall(t))
        text_white += len(re_text_white.findall(t))

        rect_black += len(re_rect_black.findall(t))
        rect_white += len(re_rect_white.findall(t))

        sdk_theme_refs += len(re_sdk_theme.findall(t))

    return FaceSignals(
        fill_black=fill_black,
        fill_white=fill_white,
        bitmap_black=bitmap_black,
        bitmap_white=bitmap_white,
        text_black=text_black,
        text_white=text_white,
        rect_black=rect_black,
        rect_white=rect_white,
        sdk_theme_refs=sdk_theme_refs,
    )


def infer_polarity(signals: FaceSignals) -> Tuple[str, str]:
    """Return (polarity, confidence)."""

    # Strongest signal: explicit full-screen fill.
    if signals.fill_black > 0 and signals.fill_white == 0:
        return "WhiteOnBlack", "high"
    if signals.fill_white > 0 and signals.fill_black == 0:
        return "BlackOnWhite", "high"

    # If both are present, pick the more common one.
    if signals.fill_black != signals.fill_white:
        return ("WhiteOnBlack", "med") if signals.fill_black > signals.fill_white else ("BlackOnWhite", "med")

    # Next: predominant draw color for bitmaps/rects/text.
    black_score = signals.bitmap_black + signals.text_black + signals.rect_black
    white_score = signals.bitmap_white + signals.text_white + signals.rect_white

    if black_score == 0 and white_score == 0:
        return "BlackOnWhite", "low"  # default

    if abs(black_score - white_score) >= 2:
        return ("WhiteOnBlack", "med") if white_score > black_score else ("BlackOnWhite", "med")

    return ("WhiteOnBlack", "low") if white_score > black_score else ("BlackOnWhite", "low")


def main() -> int:
    face_names = parse_registry_order(REGISTRY_CPP)

    rows: List[Tuple[int, str, str, str, FaceSignals]] = []
    white_on_black_ids: List[int] = []
    uses_theme_ids: List[int] = []

    for face_id, name in enumerate(face_names):
        sig = scan_signals(name)
        polarity, conf = infer_polarity(sig)

        if polarity == "WhiteOnBlack":
            white_on_black_ids.append(face_id)
        if sig.sdk_theme_refs > 0:
            uses_theme_ids.append(face_id)

        rows.append((face_id, name, polarity, conf, sig))

    # Report
    lines: List[str] = []
    lines.append("# Watchface authored polarity report\n")
    lines.append("Generated by tools/derive_watchface_polarities.py\n")
    lines.append("\n")
    lines.append("Signals scanned: fillScreen, drawBitmap, setTextColor, fillRect, UiSDK/BASE_POLARITY refs\n")
    lines.append("\n")

    for face_id, name, polarity, conf, sig in rows:
        lines.append(f"## {face_id:02d} {name}\n")
        lines.append(f"- inferred: **{polarity}** ({conf})\n")
        lines.append(
            "- counts: "
            f"fill(B)={sig.fill_black} fill(W)={sig.fill_white} | "
            f"bitmap(B)={sig.bitmap_black} bitmap(W)={sig.bitmap_white} | "
            f"text(B)={sig.text_black} text(W)={sig.text_white} | "
            f"rect(B)={sig.rect_black} rect(W)={sig.rect_white} | "
            f"sdkThemeRefs={sig.sdk_theme_refs}\n"
        )
        lines.append("\n")

    REPORT_MD.write_text("".join(lines), encoding="utf-8")

    # C++ snippet for UiSDK.cpp
    print("// --- Generated by tools/derive_watchface_polarities.py")
    print("switch (id) {")
    for face_id in white_on_black_ids:
        name = face_names[face_id]
        print(f"  case {face_id}: // {name}")
    print("    return WatchfacePolarity::WhiteOnBlack;\n")
    print("  default:")
    print("    return WatchfacePolarity::BlackOnWhite;")
    print("}")
    print("\n// Faces referencing UiSDK theme palette / BASE_POLARITY (review for double-inversion):")
    print(", ".join(str(i) for i in uses_theme_ids) if uses_theme_ids else "(none)")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
