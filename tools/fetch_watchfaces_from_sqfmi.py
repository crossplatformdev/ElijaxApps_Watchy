import hashlib
import os
import re
import shutil
import subprocess
import sys
import urllib.request
from dataclasses import dataclass
from pathlib import Path
from typing import Dict, Iterable, List, Optional, Tuple

PAGE_URL = "https://watchy.sqfmi.com/watchfaces"

REPO_HOSTS = (
    "https://github.com/",
    "https://git.sr.ht/",
)


@dataclass(frozen=True)
class SourceSpec:
    name: str
    url: str
    repo_url: str
    branch: Optional[str]
    subpath: Optional[str]


def _run(cmd: List[str], cwd: Optional[Path] = None) -> None:
    subprocess.check_call(cmd, cwd=str(cwd) if cwd else None)


def _sha1(text: str) -> str:
    return hashlib.sha1(text.encode("utf-8"), usedforsecurity=False).hexdigest()


def fetch_html(url: str) -> str:
    req = urllib.request.Request(url, headers={"User-Agent": "Mozilla/5.0"})
    with urllib.request.urlopen(req, timeout=60) as resp:
        return resp.read().decode("utf-8", errors="replace")


def extract_watchface_links(html: str) -> List[Tuple[str, str]]:
    # Extract anchor tags and keep those that look like git repos.
    anchors = re.findall(r"<a\s+[^>]*href=\"([^\"]+)\"[^>]*>(.*?)</a>", html, flags=re.IGNORECASE)

    def strip_tags(text: str) -> str:
        return re.sub(r"<[^>]+>", "", text).strip()

    items: List[Tuple[str, str]] = []
    for href, inner in anchors:
        name = strip_tags(inner)
        if not name:
            continue
        if name in ("GitHub", "Docs", "Getting Started", "Create Your Own!", "Discord", "Twitter", "Instagram", "YouTube"):
            continue
        if not href.startswith(REPO_HOSTS):
            continue
        # Ignore obvious image / non-repo links
        if any(seg in href for seg in ("/raw/", "/img/", ".png", ".gif", ".jpg", ".bmp")):
            continue
        items.append((name, href))

    # De-dupe exact duplicates while preserving order
    seen = set()
    deduped: List[Tuple[str, str]] = []
    for name, href in items:
        key = (name, href)
        if key in seen:
            continue
        seen.add(key)
        deduped.append((name, href))

    return deduped


def parse_source(name: str, url: str) -> SourceSpec:
    if url.startswith("https://github.com/"):
        m = re.match(r"https://github.com/([^/]+)/([^/]+?)(?:\.git)?(?:/.*)?$", url)
        if not m:
            raise ValueError(f"Unrecognized GitHub URL: {url}")
        owner, repo = m.group(1), m.group(2)

        # Handle /tree/<branch>/<path>
        mtree = re.match(r"https://github.com/([^/]+)/([^/]+?)(?:\.git)?/tree/([^/]+)/(.*)$", url)
        if mtree:
            owner, repo, branch, subpath = mtree.group(1), mtree.group(2), mtree.group(3), mtree.group(4)
            repo_url = f"https://github.com/{owner}/{repo}.git"
            return SourceSpec(name=name, url=url, repo_url=repo_url, branch=branch, subpath=subpath)

        repo_url = f"https://github.com/{owner}/{repo}.git"
        return SourceSpec(name=name, url=url, repo_url=repo_url, branch=None, subpath=None)

    if url.startswith("https://git.sr.ht/"):
        # SourceHut git clone URL is the same; no subpaths supported here.
        return SourceSpec(name=name, url=url, repo_url=url, branch=None, subpath=None)

    raise ValueError(f"Unsupported URL: {url}")


def sanitize_dir_name(name: str) -> str:
    # Keep existing style close to current tree: underscores instead of spaces.
    s = name.strip()
    s = s.replace(" ", "_")
    s = re.sub(r"[^A-Za-z0-9._-]+", "_", s)
    s = s.strip("_ ")
    if not s:
        s = "Watchface"
    return s


def apply_version_preference(entries: List[Tuple[str, str]]) -> List[Tuple[str, str]]:
    # If both "X" and "X 2.0" exist, keep only "X 2.0" but map it to name "X".
    normalized: Dict[str, Tuple[str, str]] = {}
    raw: Dict[str, Tuple[str, str]] = {}

    def base_name(n: str) -> Tuple[str, bool]:
        n = n.strip()
        if n.endswith("2.0") or n.endswith("2.0 "):
            return n.replace("2.0", "").strip(), True
        if n.lower().endswith(" 2.0"):
            return n[:-4].strip(), True
        return n, False

    # First pass: remember originals
    for n, u in entries:
        raw[n] = (n, u)

    # Second pass: prefer 2.0
    for n, u in entries:
        b, is20 = base_name(n)
        if b not in normalized:
            normalized[b] = (n, u)
        else:
            prev_name, prev_url = normalized[b]
            prev_is20 = base_name(prev_name)[1]
            if is20 and not prev_is20:
                normalized[b] = (n, u)

    # Emit in original order of base names as they first appeared
    emitted: List[Tuple[str, str]] = []
    seen_base = set()
    for n, _ in entries:
        b, _ = base_name(n)
        if b in seen_base:
            continue
        seen_base.add(b)
        chosen_name, chosen_url = normalized[b]
        # Map directory name to base name if 2.0 chosen
        _, chosen_is20 = base_name(chosen_name)
        final_name = b if chosen_is20 else chosen_name
        emitted.append((final_name, chosen_url))

    return emitted


def clone_repo(repo_url: str, branch: Optional[str], tmp_root: Path) -> Path:
    key = repo_url + (f"@{branch}" if branch else "")
    dst = tmp_root / _sha1(key)[:12]
    if dst.exists():
        return dst

    dst.parent.mkdir(parents=True, exist_ok=True)

    cmd = ["git", "clone", "--depth", "1"]
    if branch:
        cmd += ["--branch", branch]
    cmd += [repo_url, str(dst)]

    try:
        _run(cmd)
    except subprocess.CalledProcessError:
        if branch:
            # Retry without branch (some hosts/URLs may reject it)
            shutil.rmtree(dst, ignore_errors=True)
            cmd = ["git", "clone", "--depth", "1", repo_url, str(dst)]
            _run(cmd)
        else:
            raise

    return dst


def copy_watchface(src_repo: Path, subpath: Optional[str], dest_dir: Path) -> None:
    src_path = src_repo / subpath if subpath else src_repo
    if not src_path.exists():
        raise FileNotFoundError(f"Source path does not exist: {src_path}")

    if dest_dir.exists():
        shutil.rmtree(dest_dir)

    dest_dir.parent.mkdir(parents=True, exist_ok=True)
    shutil.copytree(src_path, dest_dir)

    # Remove any nested .git that could have been copied (usually only in repo root copies)
    git_dir = dest_dir / ".git"
    if git_dir.exists():
        shutil.rmtree(git_dir, ignore_errors=True)


def main() -> int:
    repo_root = Path(__file__).resolve().parents[1]
    # Keep third-party watchfaces out of src/ so PlatformIO doesn't compile them by default.
    watchfaces_dir = repo_root / "watchfaces"
    tmp_root = repo_root / ".tmp" / "watchfaces"

    print(f"Fetching watchface list from {PAGE_URL}...")
    html = fetch_html(PAGE_URL)
    links = extract_watchface_links(html)
    if not links:
        print("No watchface links found; page structure may have changed.")
        return 2

    links = apply_version_preference(links)

    # Build unique directory names (avoid collisions)
    used_dirs: Dict[str, int] = {}

    sources: List[Tuple[str, SourceSpec]] = []
    for name, url in links:
        try:
            spec = parse_source(name, url)
        except Exception as e:
            print(f"Skipping {name}: {e}")
            continue

        base_dir = sanitize_dir_name(name)
        n = used_dirs.get(base_dir, 0)
        used_dirs[base_dir] = n + 1
        dir_name = base_dir if n == 0 else f"{base_dir}_{n+1}"
        sources.append((dir_name, spec))

    print(f"Found {len(sources)} watchface entries. Downloading via git...")

    for i, (dir_name, spec) in enumerate(sources, start=1):
        dest = watchfaces_dir / dir_name
        try:
            repo = clone_repo(spec.repo_url, spec.branch, tmp_root)
            copy_watchface(repo, spec.subpath, dest)
            print(f"[{i}/{len(sources)}] {spec.name} -> {dest.relative_to(repo_root)}")
        except Exception as e:
            print(f"[{i}/{len(sources)}] FAILED {spec.name} ({spec.url}): {e}")

    print("Done.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
