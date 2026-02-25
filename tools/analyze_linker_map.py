#!/usr/bin/env python3
"""Summarize linked Flash/RAM contributions from a PlatformIO linker map."""

from __future__ import annotations

import argparse
import json
import re
from collections import defaultdict
from pathlib import Path


OUTPUT_SECTION = re.compile(r"^(\.[A-Za-z0-9_.]+)\s+0x[0-9a-fA-F]+\s+0x[0-9a-fA-F]+")
CONTRIBUTION = re.compile(
    r"^\s+(?:\.[^\s]+\s+)?0x[0-9a-fA-F]+\s+0x([0-9a-fA-F]+)\s+(.+)$"
)

FLASH_SECTIONS = {".flash.text", ".flash.rodata", ".flash.appdesc"}
RAM_SECTIONS = {
    ".dram0.data", ".dram0.bss", ".iram0.text", ".iram0.data",
    ".iram0.bss", ".rtc.data", ".rtc.text", ".rtc_noinit",
    ".rtc.force_fast", ".rtc.force_slow",
}


def normalize_object(value: str) -> str | None:
    value = value.strip().replace("\\", "/")
    if " (size before relaxing)" in value or value.startswith("load address"):
        return None
    if ".o" not in value and ".a(" not in value:
        return None
    return value.split()[0]


def parse_map(lines: list[str]) -> dict[str, dict[str, int]]:
    contributions: dict[str, dict[str, int]] = defaultdict(
        lambda: {"flash": 0, "ram": 0}
    )
    current_section = None
    for line in lines:
        section_match = OUTPUT_SECTION.match(line)
        if section_match:
            current_section = section_match.group(1)
            continue
        if current_section not in FLASH_SECTIONS | RAM_SECTIONS:
            continue
        contribution_match = CONTRIBUTION.match(line)
        if not contribution_match:
            continue
        object_name = normalize_object(contribution_match.group(2))
        if object_name is None:
            continue
        size = int(contribution_match.group(1), 16)
        category = "flash" if current_section in FLASH_SECTIONS else "ram"
        contributions[object_name][category] += size
    return dict(contributions)


def is_project_object(name: str) -> bool:
    return "/src/" in name or name.startswith("src/")


def ranked(contributions: dict[str, dict[str, int]], category: str,
           project_only: bool, limit: int) -> list[dict[str, object]]:
    entries = (
        {"object": name, **sizes}
        for name, sizes in contributions.items()
        if not project_only or is_project_object(name)
    )
    return sorted(entries, key=lambda entry: int(entry[category]), reverse=True)[:limit]


def build_report(map_path: Path, limit: int = 20) -> dict[str, object]:
    contributions = parse_map(map_path.read_text(errors="replace").splitlines())
    return {
        "schema_version": 1,
        "map": map_path.as_posix(),
        "linked_object_count": len(contributions),
        "accounted_flash_bytes": sum(value["flash"] for value in contributions.values()),
        "accounted_ram_bytes": sum(value["ram"] for value in contributions.values()),
        "top_project_flash": ranked(contributions, "flash", True, limit),
        "top_project_ram": ranked(contributions, "ram", True, limit),
        "top_all_flash": ranked(contributions, "flash", False, limit),
        "top_all_ram": ranked(contributions, "ram", False, limit),
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "map", type=Path,
        default=Path(".pio/build/esp32-s3-devkitc-1/firmware.map"),
        nargs="?",
    )
    parser.add_argument("--limit", type=int, default=20)
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()
    payload = json.dumps(build_report(args.map, args.limit), indent=2) + "\n"
    if args.output:
        args.output.write_text(payload, encoding="ascii", newline="\n")
    else:
        print(payload, end="")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())