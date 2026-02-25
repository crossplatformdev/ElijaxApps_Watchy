"""Parse the X-macro catalog shared by deterministic gallery firmware."""

from __future__ import annotations

import re
from dataclasses import dataclass
from pathlib import Path
from typing import Sequence


CALIBRATION_SCENE_IDS = (
    "sdk/grayscale/ramp/light",
    "sdk/grayscale/semantic-tones/light",
    "sdk/grayscale/semantic-tones/dark",
    "sdk/grayscale/widgets/light",
    "sdk/grayscale/widgets/dark",
    "sdk/grayscale/dialog/light",
    "sdk/grayscale/list/light",
    "sdk/grayscale/disabled-controls/light",
    "sdk/grayscale/graph/light",
)

WATCHFACE_SCENE_IDS = tuple(
    f"watchfaces/{watchface}/{theme}"
    for watchface in (
        "7-seg", "basic", "dos", "macpaint", "mario", "pokemon",
        "starry-horizon", "tetris",
    )
    for theme in ("light", "dark")
)

MENU_SCENE_IDS = (
    "os/menu/categories/light",
    "os/menu/clocks-sky/light",
    "os/menu/timers-focus/light",
    "os/menu/health-wellness/light",
    "os/menu/safety-first-aid/light",
    "os/menu/sensors-activity/light",
    "os/menu/everyday-tools/light",
    "os/menu/games-puzzles/light",
    "os/menu/network-web/light",
    "os/menu/bluetooth/light",
    "os/menu/watch-system/light",
)

CATALOG_LINE = re.compile(
    r"GALLERY_APP\(\s*([A-Z_]+)\s*,\s*(\d+)\s*,\s*\"([^\"]+)\"\s*,\s*"
    r"((?:\"[^\"]+\"|nullptr)(?:\s*,\s*(?:\"[^\"]+\"|nullptr)){4})"
    r"\s*\)"
)


@dataclass(frozen=True)
class GalleryApp:
    renderer: str
    tool: int
    prefix: str
    states: tuple[str, ...]

    def scene_ids(self) -> tuple[str, ...]:
        return tuple(f"{self.prefix}/{state}/light" for state in self.states)


def load_app_catalog(path: Path) -> tuple[GalleryApp, ...]:
    try:
        lines = path.read_text(encoding="ascii").splitlines()
    except OSError as error:
        raise ValueError(f"cannot read application catalog {path}: {error}") from error

    applications = []
    for line_number, line in enumerate(lines, 1):
        stripped = line.strip()
        if not stripped or stripped.startswith("//"):
            continue
        match = CATALOG_LINE.fullmatch(stripped)
        if match is None:
            raise ValueError(f"invalid catalog entry at {path}:{line_number}")

        states = []
        reached_null = False
        for token in (part.strip() for part in match.group(4).split(",")):
            if token == "nullptr":
                reached_null = True
                continue
            if reached_null:
                raise ValueError(
                    f"non-null state after nullptr at {path}:{line_number}"
                )
            states.append(token[1:-1])
        if not states:
            raise ValueError(f"application has no gallery state at {path}:{line_number}")

        applications.append(
            GalleryApp(match.group(1), int(match.group(2)), match.group(3), tuple(states))
        )

    if not applications:
        raise ValueError(f"application catalog is empty: {path}")
    prefixes = tuple(application.prefix for application in applications)
    if len(set(prefixes)) != len(prefixes):
        raise ValueError(f"application catalog contains duplicate prefixes: {path}")
    return tuple(applications)


def ordered_scene_ids(applications: Sequence[GalleryApp]) -> tuple[str, ...]:
    application_scenes = tuple(
        scene_id
        for application in applications
        for scene_id in application.scene_ids()
    )
    scene_ids = (
        CALIBRATION_SCENE_IDS + WATCHFACE_SCENE_IDS + MENU_SCENE_IDS +
        application_scenes
    )
    if len(set(scene_ids)) != len(scene_ids):
        raise ValueError("generated gallery contains duplicate scene IDs")
    return scene_ids


def render_scene_catalog(applications: Sequence[GalleryApp]) -> str:
    scene_ids = ordered_scene_ids(applications)
    return (
        "# Generated from src/demo/GalleryAppCatalog.inc; do not edit.\n"
        "# Protocol order for the deterministic light-theme gallery.\n"
        + "\n".join(scene_ids)
        + "\n"
    )