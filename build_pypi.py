#!/usr/bin/env python3
"""
Baba GUI - Build and upload to PyPI

Usage:
    python build_pypi.py [--upload] [--test]
    
Options:
    --upload    Upload to PyPI after building
    --test      Upload to TestPyPI instead of PyPI
"""
import os
import re
import sys
import shutil
import subprocess
from pathlib import Path

VERSION_FILE = Path("python/baba/__init__.py")


def get_version():
    content = VERSION_FILE.read_text(encoding="utf-8")
    match = re.search(r'__version__\s*=\s*"(\d+\.\d+\.\d+)"', content)
    return match.group(1) if match else "0.0.0"


def bump_version():
    content = VERSION_FILE.read_text(encoding="utf-8")
    match = re.search(r'(__version__\s*=\s*"(\d+\.\d+\.)(\d+)")', content)
    if not match:
        print("ERROR: cannot parse version")
        sys.exit(1)
    
    old_version = match.group(2) + match.group(3)
    new_version = match.group(2) + str(int(match.group(3)) + 1)
    
    new_content = content.replace(match.group(1), f'__version__ = "{new_version}"')
    VERSION_FILE.write_text(new_content, encoding="utf-8")
    
    print(f"  {old_version} -> {new_version}")
    return new_version


def clean():
    print("[2/6] Cleaning old builds...")
    dirs_to_remove = ["dist", "build", "baba_gui.egg-info"]
    for d in dirs_to_remove:
        if Path(d).exists():
            shutil.rmtree(d)
    
    lib_dir = Path("python/baba/lib")
    include_dir = Path("python/baba/include")
    
    if lib_dir.exists():
        shutil.rmtree(lib_dir)
    if include_dir.exists():
        shutil.rmtree(include_dir)
    
    lib_dir.mkdir(parents=True, exist_ok=True)
    include_dir.mkdir(parents=True, exist_ok=True)


def build_native():
    print("[3/6] Building C library...")
    
    os_name = sys.platform
    build_dir = Path("build")
    
    cmake_args = [
        "cmake", "-B", str(build_dir),
        "-DCMAKE_BUILD_TYPE=Release",
        "-DBABA_BUILD_EXAMPLES=OFF",
        "-DBABA_BUILD_TESTS=OFF",
    ]
    
    if os_name == "darwin":
        cmake_args.extend(["-G", "Ninja"])
        cmake_args.append("-DCMAKE_OSX_ARCHITECTURES=arm64;x86_64")
    elif os_name == "win32":
        if shutil.which("ninja"):
            cmake_args.extend(["-G", "Ninja"])
    else:
        cmake_args.extend(["-G", "Ninja"])
    
    subprocess.run(cmake_args, check=True)
    subprocess.run(["cmake", "--build", str(build_dir), "--config", "Release"], check=True)
    
    lib_dir = Path("python/baba/lib")
    
    if os_name == "darwin":
        lib_src = build_dir / "libbaba.a"
        lib_dst = lib_dir / "libbaba_macos.a"
        if lib_src.exists():
            shutil.copy(lib_src, lib_dst)
            print(f"  Copied: {lib_dst}")
    elif os_name == "win32":
        lib_src = build_dir / "libbaba.a"
        if not lib_src.exists():
            lib_src = build_dir / "Release" / "baba.lib"
        if lib_src.exists():
            lib_dst = lib_dir / "libbaba_windows.a" if lib_src.suffix == ".a" else lib_dir / "baba_windows.lib"
            shutil.copy(lib_src, lib_dst)
            print(f"  Copied: {lib_dst}")
    else:
        lib_src = build_dir / "libbaba.a"
        lib_dst = lib_dir / "libbaba_linux.a"
        if lib_src.exists():
            shutil.copy(lib_src, lib_dst)
            print(f"  Copied: {lib_dst}")


def copy_headers():
    print("[4/6] Copying C headers...")
    
    include_dir = Path("python/baba/include")
    src_include = Path("dist/include")
    
    if src_include.exists():
        for item in src_include.iterdir():
            if item.is_dir():
                dst = include_dir / item.name
                dst.mkdir(exist_ok=True)
                for f in item.iterdir():
                    if f.suffix == ".h":
                        shutil.copy(f, dst / f.name)
            elif item.suffix == ".h":
                shutil.copy(item, include_dir / item.name)
    
    src_dir = Path("src")
    for h in src_dir.glob("**/*.h"):
        rel_path = h.relative_to(src_dir)
        dst = include_dir / rel_path
        dst.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy(h, dst)
    
    print(f"  Headers copied to {include_dir}")


def build_package():
    print("[5/6] Building Python package...")
    
    subprocess.run([sys.executable, "-m", "pip", "install", "--upgrade", "build", "twine", "-q"], check=True)
    subprocess.run([sys.executable, "-m", "build"], check=True)
    subprocess.run([sys.executable, "-m", "twine", "check", "dist/*"], check=True)


def upload(test=False):
    print("[6/6] Uploading to PyPI...")
    
    if test:
        subprocess.run([
            sys.executable, "-m", "twine", "upload",
            "--repository", "testpypi",
            "dist/*"
        ], check=True)
    else:
        subprocess.run([sys.executable, "-m", "twine", "upload", "dist/*"], check=True)


def main():
    print("=== Baba PyPI Upload ===\n")
    
    do_upload = "--upload" in sys.argv
    use_test = "--test" in sys.argv
    
    print("[1/6] Bumping patch version...")
    version = bump_version()
    
    clean()
    build_native()
    copy_headers()
    build_package()
    
    if do_upload:
        upload(test=use_test)
    else:
        print("[6/6] Skipping upload (--upload not specified)")
    
    print("\n=== Done! ===")
    print(f"  Package: baba-gui=={version}")
    print(f"  Install: pip install baba-gui")
    print()


if __name__ == "__main__":
    main()