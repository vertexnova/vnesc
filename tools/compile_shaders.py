#!/usr/bin/env python3
"""vnesc offline shader compiler — thin wrapper around vnesc_shader_compiler.

Usage:
    compile_shaders.py --manifest <glob-or-path> --output <dir>
                       [--cache <dir>] [--compiler <path>]
                       [--parallel <N>] [--dry-run] [--verbose]
"""

import argparse
import glob
import os
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor, as_completed
from pathlib import Path


def find_compiler(override: str | None) -> Path | None:
    """Search: --compiler arg → PATH → ./build/bin/ → sibling vnesc/build/bin/"""
    if override:
        p = Path(override)
        if p.is_file():
            return p
        sys.stderr.write(f"error: --compiler path not found: {override}\n")
        return None

    name = "vnesc_shader_compiler"
    # 1. PATH
    import shutil
    found = shutil.which(name)
    if found:
        return Path(found)

    # 2. ./build/bin/ relative to CWD
    candidates = [
        Path("build") / "bin" / name,
        Path("build") / name,
        Path("../vnesc/build/bin") / name,
        Path("../vnesc/build") / name,
    ]
    for c in candidates:
        if c.is_file():
            return c

    return None


def compile_one(
    compiler: Path,
    manifest: Path,
    output: Path,
    cache: Path | None,
    dry_run: bool,
    verbose: bool,
) -> tuple[bool, str, str]:
    """Run vnesc_shader_compiler for one manifest. Returns (ok, manifest_str, error_msg)."""
    cmd = [str(compiler), "--manifest", str(manifest), "--output", str(output)]
    if cache:
        cmd += ["--cache", str(cache)]

    if dry_run:
        print(f"[dry-run] {' '.join(cmd)}")
        return True, str(manifest), ""

    if verbose:
        print(f"compiling: {manifest}")

    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.returncode == 0:
        return True, str(manifest), ""
    return False, str(manifest), result.stderr.strip()


def expand_manifests(patterns: list[str]) -> list[Path]:
    paths: list[Path] = []
    for pattern in patterns:
        expanded = glob.glob(pattern, recursive=True)
        if expanded:
            paths.extend(Path(p) for p in expanded)
        else:
            p = Path(pattern)
            if p.is_file():
                paths.append(p)
            else:
                sys.stderr.write(f"warning: no files matched: {pattern}\n")
    return paths


def main() -> int:
    parser = argparse.ArgumentParser(
        description="vnesc offline shader compiler — thin wrapper around vnesc_shader_compiler"
    )
    parser.add_argument(
        "--manifest", "-m",
        nargs="+",
        required=True,
        metavar="GLOB",
        help="One or more manifest paths or glob patterns (e.g. 'shaders/**/*.pipeline.json')",
    )
    parser.add_argument(
        "--output", "-o",
        required=True,
        metavar="DIR",
        help="Output directory for compiled .vneshader bundles",
    )
    parser.add_argument(
        "--cache", "-c",
        metavar="DIR",
        help="Optional shader artifact cache directory",
    )
    parser.add_argument(
        "--compiler",
        metavar="PATH",
        help="Explicit path to vnesc_shader_compiler binary",
    )
    parser.add_argument(
        "--parallel", "-j",
        type=int,
        default=max(1, (os.cpu_count() or 2) // 2),
        metavar="N",
        help="Number of parallel compilation jobs (default: half the CPU count)",
    )
    parser.add_argument("--dry-run", action="store_true", help="Print commands without running them")
    parser.add_argument("--verbose", "-v", action="store_true", help="Print each manifest being compiled")

    args = parser.parse_args()

    compiler = find_compiler(args.compiler)
    if compiler is None:
        sys.stderr.write(
            "error: vnesc_shader_compiler not found.\n"
            "  Build it with: cmake --build <build-dir> --target vnesc_shader_compiler\n"
            "  Or pass --compiler <path>.\n"
        )
        return 1

    manifests = expand_manifests(args.manifest)
    if not manifests:
        sys.stderr.write("error: no manifest files found\n")
        return 1

    output_dir = Path(args.output)
    cache_dir = Path(args.cache) if args.cache else None

    if args.verbose or args.dry_run:
        print(f"compiler: {compiler}")
        print(f"manifests: {len(manifests)}")
        print(f"output:   {output_dir}")

    failures: list[tuple[str, str]] = []

    with ThreadPoolExecutor(max_workers=args.parallel) as pool:
        futures = {
            pool.submit(compile_one, compiler, m, output_dir, cache_dir, args.dry_run, args.verbose): m
            for m in manifests
        }
        for future in as_completed(futures):
            ok, manifest_str, error = future.result()
            if not ok:
                failures.append((manifest_str, error))
                sys.stderr.write(f"FAILED: {manifest_str}\n{error}\n")

    if failures:
        sys.stderr.write(f"\n{len(failures)} of {len(manifests)} manifest(s) failed.\n")
        return 1

    if not args.dry_run:
        print(f"compiled {len(manifests)} manifest(s) → {output_dir}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
