#!/usr/bin/env python3
import os
import subprocess
import sys
import glob

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
    pico_sdk_path = "C:/Users/710_300704/.pico-sdk"
    toolchain_path = os.path.join(pico_sdk_path, "toolchain", "13_3_Rel1", "arm-none-eabi", "include")

    # Fix path for Windows
    toolchain_path = os.path.normpath(toolchain_path)

    for file in source_files:
        print(f"Analyzing {file}...")
        command = [
            "clang-tidy",
            "-p", compile_db_path,
            "main.cpp"
        ]
        command.append(file)
        print(command)
        try:
            subprocess.run(command, check=True, capture_output=True, text=True)
        except subprocess.CalledProcessError as e:
            print(f"Error running clang-tidy: {e}")
            print(f"Return code: {e.returncode}")
            print(f"stdout: {e.stdout}")
            print(f"stderr: {e.stderr}")

if __name__ == "__main__":
    main()