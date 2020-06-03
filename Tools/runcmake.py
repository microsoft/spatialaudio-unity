# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.
#!/usr/bin/env python3
import sys
# Prevent Python from generating .pyc files
sys.dont_write_bytecode = True

import argparse
import constants
import os
import subprocess
import oshelpers
import githelpers

generator_vs2019 = "\"Visual Studio 16 2019\""

windows_arm_cmake = ["-DCMAKE_GENERATOR_PLATFORM=ARM"]
windows_arm64_cmake = ["-DCMAKE_GENERATOR_PLATFORM=ARM64"]
windowsstore_x86_cmake = ["-DCMAKE_SYSTEM_NAME=WindowsStore", "-DCMAKE_SYSTEM_VERSION=10.0"]
windowsstore_x64_cmake = ["-DCMAKE_SYSTEM_NAME=WindowsStore", "-DCMAKE_SYSTEM_VERSION=10.0"]
windowsstore_arm_cmake = ["-DCMAKE_GENERATOR_PLATFORM=ARM", "-DCMAKE_SYSTEM_NAME=WindowsStore", "-DCMAKE_SYSTEM_VERSION=10.0"]
windowsstore_arm64_cmake = ["-DCMAKE_GENERATOR_PLATFORM=ARM64", "-DCMAKE_SYSTEM_NAME=WindowsStore", "-DCMAKE_SYSTEM_VERSION=10.0"]

# os specific cmake command
def call_cmake():
    return "cmake -G "

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--windows", help="Generate Windows desktop configurations", action='store_true')
    parser.add_argument("--windowsstore", help="Generate WindowsStore (UWP) configurations", action='store_true')
    parser.add_argument("--notest", help="Skip generation of test configurations", action='store_true')
    parser.add_argument("-v", "--version", help="Semantic version string", type=str.lower)
    args = parser.parse_args()

    cmake_windows = False
    cmake_windowsstore = False
    cmake_tests = True

    # No args provides, generate all configs
    if len(sys.argv) == 1:
        cmake_windows = True
        cmake_windowsstore = True
        cmake_tests = True
    # Generate Windows desktop configs
    if args.windows:
        cmake_windows = True
    # Generate UWP configs
    if args.windowsstore:
        cmake_windowsstore = True
    # Turn off test config generation
    if args.notest:
        cmake_tests = False

    git_root = oshelpers.fixpath(githelpers.get_root())
    build_dir = oshelpers.fixpath(git_root, constants.build_root)
    print("Creating build dirs under '%s'" %build_dir)

    # Pass version (if specified) to CMake
    product_version_cmake = ''
    if args.version:
        product_version_cmake = "-DPRODUCT_VERSION=" + args.version

    windows_win32_cmake = [generator_vs2019, "-A Win32", product_version_cmake]
    windows_x64_cmake = [generator_vs2019, "-A x64", product_version_cmake]
    windows_arm_cmake[:0] = [generator_vs2019, "-A ARM", product_version_cmake]
    windows_arm64_cmake[:0] = [generator_vs2019, "-A ARM64", product_version_cmake]

    if (cmake_windows):
        create_build_folder_for_platform_architecture(build_dir, "Windows", "x86", windows_win32_cmake, cmake_tests, git_root)
        create_build_folder_for_platform_architecture(build_dir, "Windows", "x64", windows_x64_cmake, cmake_tests, git_root)
        create_build_folder_for_platform_architecture(build_dir, "Windows", "arm", windows_arm_cmake, cmake_tests, git_root)
        create_build_folder_for_platform_architecture(build_dir, "Windows", "arm64", windows_arm64_cmake, cmake_tests, git_root)
    if (cmake_windowsstore):
        create_build_folder_for_platform_architecture(build_dir, "WindowsStore", "x86", windows_win32_cmake, cmake_tests, git_root)
        create_build_folder_for_platform_architecture(build_dir, "WindowsStore", "x64", windows_x64_cmake, cmake_tests, git_root)
        create_build_folder_for_platform_architecture(build_dir, "WindowsStore", "ARM", windows_arm_cmake, cmake_tests, git_root)
        create_build_folder_for_platform_architecture(build_dir, "WindowsStore", "ARM64", windows_arm64_cmake, cmake_tests, git_root)

def create_build_folder_for_platform_architecture(build_dir, system, arch, cmake_options, cmake_tests, git_root):
    folder = oshelpers.fixpath(build_dir, system, arch)

    # Create the folder if it doesn't exist
    if not os.path.exists(folder):
        print("Creating dir %s" %folder)
        os.makedirs(folder)

    cmake_test_options = " -DCMAKE_TEST=TRUE"
    if (cmake_tests == False):
        cmake_test_options = " -DCMAKE_TEST=FALSE"

    # Run cmake with the build directory as the working dir and git root as the cmake root
    cmake_command = call_cmake() + " ".join(cmake_options) + cmake_test_options + " " + git_root

    print("Executing command: %s" %cmake_command)
    print("Executing CMake in %s" %folder)
    subprocess.run(cmake_command, cwd = folder, check = True, shell = True)

if __name__ == '__main__':
    main()
