#!/usr/bin/env python3
import os
import subprocess
import sys
import glob

def find_all_header_paths(root_dir):
    """Recursively searches for header files in a directory and returns their paths."""
    header_paths = []
    for dirpath, dirnames, filenames in os.walk(root_dir):
        for filename in filenames:
            if filename.endswith((".h", ".hpp")):
                header_paths.append(dirpath)
                break  # Only add the directory once if it contains a header
    return header_paths

def main():
    compile_db_path = "build/compile_commands.json"
    if not os.path.exists(compile_db_path):
        print(f"Error: {compile_db_path} not found! Run CMake first.")
        return 1

    # Find all C/C++ source files
    source_files = glob.glob("**/*.cpp", recursive=True)
    source_files.extend(glob.glob("**/*.c", recursive=True))

    # Skip files in build directory
    source_files = [f for f in source_files if not f.startswith("build/")]

    print(f"Running clang-tidy on {len(source_files)} files...")

    # Find header paths
    pico_sdk_path = "C:/Users/Kuba/.pico-sdk"
    header_paths = find_all_header_paths(pico_sdk_path)

    for file in source_files:
        print(f"Analyzing {file}...")
        command = [
            "clang-tidy",
            "-p", compile_db_path,
        ]
        for path in header_paths:
            command.append(f"--extra-arg=-I{path}")
        command.append(file)
        subprocess.run(command)

if __name__ == "__main__":
    main()