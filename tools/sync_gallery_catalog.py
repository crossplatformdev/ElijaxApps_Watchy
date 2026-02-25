#!/usr/bin/env python3
"""Synchronize gallery scene IDs and README screenshot columns."""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path
from typing import Sequence

try:
    from tools.gallery_catalog import (
        GalleryApp,
        load_app_catalog,
        ordered_scene_ids,
        render_scene_catalog,
    )
except ModuleNotFoundError:
    from gallery_catalog import (  # type: ignore[no-redef]
        GalleryApp,
        load_app_catalog,
        ordered_scene_ids,
        render_scene_catalog,
    )


README_HEADER = (
    "| Application | What it does | Screenshot 1 | Screenshot 2 | "
    "Screenshot 3 | Screenshot 4 | Screenshot 5 |"
)
README_SEPARATOR = "| --- | --- | --- | --- | --- | --- | --- |"
SCREENSHOT_PATH = re.compile(r"docs/gallery/png/([^\s)]+)\.png")


def update_readme(source: str, applications: Sequence[GalleryApp]) -> str:
    applications_by_prefix = {
        application.prefix: application for application in applications
    }
    updated_prefixes = set()
    header_count = 0
    expecting_separator = False
    output = []

    for line in source.splitlines():
        if line.startswith("| Application | What it does | Screenshot"):
            output.append(README_HEADER)
            header_count += 1
            expecting_separator = True
            continue
        if expecting_separator:
            if not line.startswith("| --- | --- |"):
                raise ValueError("README application table has no separator row")
            output.append(README_SEPARATOR)
            expecting_separator = False
            continue

        path_match = SCREENSHOT_PATH.search(line)
        if path_match is None or not line.startswith("| **"):
            output.append(line)
            continue

        relative_path = path_match.group(1)
        matching_prefixes = tuple(
            prefix
            for prefix in applications_by_prefix
            if relative_path.startswith(f"{prefix}/")
        )
        if len(matching_prefixes) != 1:
            raise ValueError(f"cannot map README screenshot path: {relative_path}")
        application = applications_by_prefix[matching_prefixes[0]]

        cells = [cell.strip() for cell in line.strip().strip("|").split("|")]
        if len(cells) < 3:
            raise ValueError(f"invalid README application row: {line}")
        display_name = cells[0].removeprefix("**").removesuffix("**")
        screenshots = [
            f"![{display_name} - {state.replace('-', ' ')}]"
            f"(docs/gallery/png/{application.prefix}/{state}/light.png)"
            for state in application.states
        ]
        screenshots.extend([""] * (5 - len(screenshots)))
        output.append(
            "| " + " | ".join((cells[0], cells[1], *screenshots)) + " |"
        )
        updated_prefixes.add(application.prefix)

    if expecting_separator:
        raise ValueError("README ends inside an application table header")
    if header_count != 10:
        raise ValueError(f"expected 10 application tables, found {header_count}")
    missing = set(applications_by_prefix) - updated_prefixes
    if missing:
        raise ValueError(
            "README has no application row for: " + ", ".join(sorted(missing))
        )
    return "\n".join(output) + "\n"


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true", help="fail if outputs are stale")
    parser.add_argument(
        "--repository", type=Path, default=Path(__file__).resolve().parents[1]
    )
    return parser


def main(argv: Sequence[str] | None = None) -> int:
    args = build_parser().parse_args(argv)
    repository = args.repository.resolve()
    source_path = repository / "src/demo/GalleryAppCatalog.inc"
    scene_path = repository / "tools/gallery_scene_ids.txt"
    readme_path = repository / "readme.md"

    try:
        applications = load_app_catalog(source_path)
        scene_text = render_scene_catalog(applications)
        readme_text = update_readme(
            readme_path.read_text(encoding="utf-8"), applications
        )
    except (OSError, ValueError) as error:
        print(f"sync_gallery_catalog: error: {error}", file=sys.stderr)
        return 1

    stale = []
    if scene_path.read_text(encoding="ascii") != scene_text:
        stale.append(scene_path)
    if readme_path.read_text(encoding="utf-8") != readme_text:
        stale.append(readme_path)
    if args.check:
        if stale:
            print("Gallery outputs are stale:", file=sys.stderr)
            for path in stale:
                print(f"  {path.relative_to(repository)}", file=sys.stderr)
            return 1
    else:
        scene_path.write_text(scene_text, encoding="ascii", newline="\n")
        readme_path.write_text(readme_text, encoding="utf-8", newline="\n")

    print(
        f"Gallery catalog: {len(applications)} applications, "
        f"{len(ordered_scene_ids(applications))} scenes"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())