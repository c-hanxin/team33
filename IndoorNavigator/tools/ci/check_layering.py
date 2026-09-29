#!/usr/bin/env python3
"""
CI Layering Check Script for SIT Block E2 Indoor Navigation Project.
Verifies that dependencies only point down:
  shell -> app -> core systems -> sim

Rule:
  source/sim/ includes NO OpenGL headers, NO window/platform headers,
  and NO dependencies on app/, ui/, or platform/.
"""

import sys
from pathlib import Path
import re

FORBIDDEN_SIM_PATTERNS = [
    # Platform / Windowing headers
    re.compile(r'#include\s*[<"](?:windows\.h|jni\.h|android/|SDL|SDL2|GLFW|X11)[>"]'),
    # OpenGL / Rendering headers
    re.compile(r'#include\s*[<"](?:GLES[23]/|GL/|glad/|KHR/)[>"]'),
    # Upper-layer dependencies (sim must not know about app, ui, render, or platform)
    re.compile(r'#include\s*[<"](?:app/|ui/|render/|platform/)[>"]'),
]

def check_sim_layer(sim_dir: Path) -> bool:
    failed = False
    print(f"[CI Layering Check] Scanning '{sim_dir}'...")

    for file_path in sim_dir.rglob("*"):
        if file_path.suffix in [".h", ".hpp", ".cpp"]:
            with open(file_path, "r", encoding="utf-8", errors="ignore") as f:
                for line_num, line in enumerate(f, start=1):
                    for pattern in FORBIDDEN_SIM_PATTERNS:
                        if pattern.search(line):
                            print(
                                f"[VIOLATION] {file_path.name}:{line_num} violates layering rule: '{line.strip()}'"
                            )
                            failed = True

    if not failed:
        print("[SUCCESS] All files in source/sim/ satisfy architectural layering constraints.")
    return not failed

def main():
    root_dir = Path(__file__).resolve().parent.parent.parent
    sim_dir = root_dir / "source" / "sim"

    if not sim_dir.exists():
        print(f"[ERROR] Directory not found: {sim_dir}")
        sys.exit(1)

    passed = check_sim_layer(sim_dir)
    sys.exit(0 if passed else 1)

if __name__ == "__main__":
    main()
