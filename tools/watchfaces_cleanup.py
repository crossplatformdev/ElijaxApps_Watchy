import os
import re
from dataclasses import dataclass

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
SRC_DIR = os.path.join(ROOT, "src")
WATCHY_DIR = os.path.join(SRC_DIR, "watchy")
GLOBAL_SETTINGS = os.path.join(SRC_DIR, "settings", "settings.h")

WATCHFACE_ROOTS = [
    os.path.join(ROOT, "watchfaces"),
    os.path.join(SRC_DIR, "watchfaces"),
]

SOURCE_EXTS = {".h", ".hpp", ".cpp", ".cxx", ".ino"}

INCLUDE_RE = re.compile(
    r"^(?P<indent>\s*)#include\s*[<\"](?P<name>[^>\"]+)[>\"](?P<trail>\s*(?://.*)?)\s*$"
)

DEFINE_RE = re.compile(r"^\s*#define\s+(?P<name>[A-Za-z_][A-Za-z0-9_]*)\b(?P<rest>.*)$")


@dataclass
class DefineLine:
    name: str
    rest: str  # includes leading whitespace and comments as-is


def to_fwd(path: str) -> str:
    return path.replace("\\", "/")


def rel_include(from_dir: str, target_file: str) -> str:
    rel = os.path.relpath(target_file, start=from_dir)
    return to_fwd(rel)


def list_watchy_headers() -> set[str]:
    headers: set[str] = set()
    for name in os.listdir(WATCHY_DIR):
        if name.lower().endswith(".h"):
            headers.add(name)
    return headers


def iter_source_files(root_dir: str):
    if not os.path.isdir(root_dir):
        return
    for dirpath, dirnames, filenames in os.walk(root_dir):
        # Skip vendor/build dirs if any
        base = os.path.basename(dirpath)
        if base in {".git", ".pio"}:
            dirnames[:] = []
            continue
        for filename in filenames:
            _, ext = os.path.splitext(filename)
            if ext.lower() in SOURCE_EXTS:
                yield os.path.join(dirpath, filename)


def iter_settings_files():
    for wf_root in WATCHFACE_ROOTS:
        for dirpath, dirnames, filenames in os.walk(wf_root):
            base = os.path.basename(dirpath)
            if base in {".git", ".pio"}:
                dirnames[:] = []
                continue
            for filename in filenames:
                if filename.lower() == "settings.h":
                    yield os.path.join(dirpath, filename)


def read_text(path: str) -> str:
    with open(path, "r", encoding="utf-8", errors="surrogateescape", newline="") as f:
        return f.read()


def write_text(path: str, text: str) -> None:
    with open(path, "w", encoding="utf-8", errors="surrogateescape", newline="") as f:
        f.write(text)


def collect_global_defines(settings_text: str) -> set[str]:
    names: set[str] = set()
    for line in settings_text.splitlines():
        m = DEFINE_RE.match(line)
        if m:
            names.add(m.group("name"))
    return names


def collect_watchface_defines(settings_text: str) -> list[DefineLine]:
    defines: list[DefineLine] = []
    for line in settings_text.splitlines():
        m = DEFINE_RE.match(line)
        if not m:
            continue
        name = m.group("name")
        if name == "SETTINGS_H":
            continue
        rest = m.group("rest")
        defines.append(DefineLine(name=name, rest=rest))
    return defines


def merge_defines_into_global(global_path: str) -> int:
    global_text = read_text(global_path)
    existing = collect_global_defines(global_text)

    merged: list[DefineLine] = []
    seen: set[str] = set()

    for settings_path in iter_settings_files():
        # Don't read the global settings itself if it happens to match
        if os.path.abspath(settings_path) == os.path.abspath(global_path):
            continue
        text = read_text(settings_path)
        for d in collect_watchface_defines(text):
            if d.name in existing:
                continue
            if d.name in seen:
                continue
            seen.add(d.name)
            merged.append(d)

    if not merged:
        return 0

    insert_block_lines: list[str] = []
    insert_block_lines.append("\n// ---- Watchface legacy settings (auto-merged) ----")
    for d in merged:
        insert_block_lines.append(f"#ifndef {d.name}")
        # Keep original spacing/comments after the name
        insert_block_lines.append(f"#define {d.name}{d.rest}")
        insert_block_lines.append(f"#endif")

    insert_block = "\n".join(insert_block_lines) + "\n"

    # Insert before the last #endif
    lines = global_text.splitlines(keepends=True)
    last_endif = None
    for i in range(len(lines) - 1, -1, -1):
        if lines[i].lstrip().startswith("#endif"):
            last_endif = i
            break
    if last_endif is None:
        raise RuntimeError("Global settings.h has no #endif")

    new_lines = lines[:last_endif] + [insert_block] + lines[last_endif:]
    write_text(global_path, "".join(new_lines))
    return len(merged)


def rewrite_includes_in_file(path: str, watchy_headers: set[str]) -> bool:
    text = read_text(path)
    changed = False
    out_lines: list[str] = []
    from_dir = os.path.dirname(path)

    for line in text.splitlines(keepends=True):
        m = INCLUDE_RE.match(line.rstrip("\r\n"))
        if not m:
            out_lines.append(line)
            continue

        name = m.group("name").strip()
        trail = m.group("trail") or ""
        indent = m.group("indent") or ""

        target = None
        if name == "settings.h":
            target = GLOBAL_SETTINGS
        elif name in watchy_headers:
            target = os.path.join(WATCHY_DIR, name)

        if target is None:
            out_lines.append(line)
            continue

        rel = rel_include(from_dir, target)
        new_line = f"{indent}#include \"{rel}\"{trail}\n"
        out_lines.append(new_line)
        if new_line != line.replace("\r\n", "\n"):
            changed = True

    if changed:
        write_text(path, "".join(out_lines))
    return changed


def rewrite_watchface_includes() -> tuple[int, int]:
    watchy_headers = list_watchy_headers()
    changed_files = 0
    visited_files = 0

    for wf_root in WATCHFACE_ROOTS:
        for path in iter_source_files(wf_root):
            visited_files += 1
            if rewrite_includes_in_file(path, watchy_headers):
                changed_files += 1

    return visited_files, changed_files


def delete_watchface_settings_files() -> int:
    deleted = 0
    for path in list(iter_settings_files()):
        # Never delete the global settings
        if os.path.abspath(path) == os.path.abspath(GLOBAL_SETTINGS):
            continue
        try:
            os.remove(path)
            deleted += 1
        except OSError:
            pass
    return deleted


def main() -> int:
    print("[1/4] Merging watchface settings into src/settings/settings.h ...")
    merged_count = merge_defines_into_global(GLOBAL_SETTINGS)
    print(f"Merged {merged_count} missing #define(s)")

    print("[2/4] Rewriting watchface includes to relative src/watchy/ headers ...")
    visited, changed = rewrite_watchface_includes()
    print(f"Visited {visited} source file(s); changed {changed} file(s)")

    print("[3/4] Deleting watchfaces/**/settings.h ...")
    deleted = delete_watchface_settings_files()
    print(f"Deleted {deleted} settings.h file(s)")

    print("[4/4] Done")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
