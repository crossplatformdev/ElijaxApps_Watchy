#!/usr/bin/env python3
"""Fix watchface sources that accidentally contain literal '\n' escape sequences.

Some imported watchface .cpp files contain lines like:
  \n\n#ifdef WATCHY_STANDALONE_WATCHFACE\nvoid setup(){
  ...
  \n#endif\n
Those backslash characters make the file invalid C/C++.

This script rewrites those sequences into real newlines.
"""

from __future__ import annotations

import os
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
WATCHFACES_ROOT = ROOT / "src" / "watchfaces"


def fix_file(path: Path) -> bool:
    try:
        original = path.read_text(encoding="utf-8", errors="ignore")
    except OSError:
        return False

    updated = original

    # Convert escaped newlines around the standalone block into real newlines.
    updated = updated.replace(
        "\\n\\n#ifdef WATCHY_STANDALONE_WATCHFACE\\n",
        "\n\n#ifdef WATCHY_STANDALONE_WATCHFACE\n",
    )
    updated = updated.replace(
        "\\n#endif\\n",
        "\n#endif\n",
    )

    if updated == original:
        return False

    path.write_text(updated, encoding="utf-8", newline="\n")
    return True


def main() -> int:
    if not WATCHFACES_ROOT.exists():
        print(f"Not found: {WATCHFACES_ROOT}")
        return 2

    changed = 0
    visited = 0

    for path in WATCHFACES_ROOT.rglob("*"):
        if not path.is_file():
            continue
        if path.suffix.lower() not in {".c", ".cc", ".cpp", ".cxx", ".h", ".hpp", ".hh"}:
            continue

        visited += 1
        if fix_file(path):
            changed += 1

    print(f"Visited {visited} file(s); fixed {changed} file(s)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
