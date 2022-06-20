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

# Android settings
android_armeabi_v7a_release_cmake = ["\"MinGW Makefiles\"", "-DCMAKE_BUILD_TYPE=RelWithDebInfo", "-DCMAKE_TOOLCHAIN_FILE=" + constants.android_toolchain, "-DCMAKE_MAKE_PROGRAM=" + constants.android_make, "-DANDROID_ABI=armeabi-v7a", "-DANDROID_ARM_NEON=TRUE", "-DANDROID_PLATFORM_LEVEL=android-23", "-DANDROID_TOOLCHAIN=clang"]
android_armeabi_v7a_debug_cmake = ["\"MinGW Makefiles\"", "-DCMAKE_BUILD_TYPE=Debug", "-DCMAKE_TOOLCHAIN_FILE=" + constants.android_toolchain, "-DCMAKE_MAKE_PROGRAM=" + constants.android_make, "-DANDROID_ABI=armeabi-v7a", "-DANDROID_ARM_NEON=TRUE", "-DANDROID_PLATFORM_LEVEL=android-23", "-DANDROID_TOOLCHAIN=clang"]
android_arm64_v8a_release_cmake = ["\"MinGW Makefiles\"", "-DCMAKE_BUILD_TYPE=RelWithDebInfo", "-DCMAKE_TOOLCHAIN_FILE=" + constants.android_toolchain, "-DCMAKE_MAKE_PROGRAM=" + constants.android_make, "-DANDROID_ABI=arm64-v8a", "-DANDROID_ARM_NEON=TRUE", "-DANDROID_PLATFORM_LEVEL=android-23", "-DANDROID_TOOLCHAIN=clang"]
android_arm64_v8a_debug_cmake = ["\"MinGW Makefiles\"", "-DCMAKE_BUILD_TYPE=Debug", "-DCMAKE_TOOLCHAIN_FILE=" + constants.android_toolchain, "-DCMAKE_MAKE_PROGRAM=" + constants.android_make, "-DANDROID_ABI=arm64-v8a", "-DANDROID_ARM_NEON=TRUE", "-DANDROID_PLATFORM_LEVEL=android-23", "-DANDROID_TOOLCHAIN=clang"]
android_x86_release_cmake = ["\"MinGW Makefiles\"", "-DCMAKE_BUILD_TYPE=RelWithDebInfo", "-DCMAKE_TOOLCHAIN_FILE=" + constants.android_toolchain, "-DCMAKE_MAKE_PROGRAM=" + constants.android_make, "-DANDROID_ABI=x86", "-DANDROID_PLATFORM_LEVEL=android-23", "-DANDROID_TOOLCHAIN=clang"]
android_x86_debug_cmake = ["\"MinGW Makefiles\"", "-DCMAKE_BUILD_TYPE=Debug", "-DCMAKE_TOOLCHAIN_FILE=" + constants.android_toolchain, "-DCMAKE_MAKE_PROGRAM=" + constants.android_make, "-DANDROID_ABI=x86", "-DANDROID_PLATFORM_LEVEL=android-23", "-DANDROID_TOOLCHAIN=clang"]


# os specific cmake command
def call_cmake():
    return "cmake -G "

def install_nuget_package():
    tools_dir = oshelpers.fixpath(githelpers.get_root(), 'tools')
    externals_root = oshelpers.fixpath(githelpers.get_root(), "Source", "External")
    nuget_restore_command = [
        "nuget",
        "restore",
        oshelpers.fixpath(tools_dir, "packages.config"),
        "-PackagesDirectory",
        externals_root,
        "-ConfigFile",
        oshelpers.fixpath(tools_dir, "nuget.config")
    ]
    subprocess.check_call(nuget_restore_command, cwd=githelpers.get_root())

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--windows", help="Generate Windows desktop configurations", action='store_true')
    parser.add_argument("--windowsstore", help="Generate WindowsStore (UWP) configurations", action='store_true')
    parser.add_argument("--android", help="Generate Android configurations", action='store_true')
    parser.add_argument("--notest", help="Skip generation of test configurations", action='store_true')
    parser.add_argument("-v", "--version", help="Semantic version string", type=str.lower)
    args = parser.parse_args()

    cmake_windows = False
    cmake_windowsstore = False
    cmake_android = False
    cmake_tests = True

    # No args provides, generate all configs
    if len(sys.argv) == 1:
        cmake_windows = True
        cmake_windowsstore = True
        cmake_android = True
        cmake_tests = True
    # Generate Windows desktop configs
    if args.windows:
        cmake_windows = True
    # Generate UWP configs
    if args.windowsstore:
        cmake_windowsstore = True
    # Generate Andrdid configs
    if args.android:
        cmake_android = True
    # Turn off test config generation
    if args.notest:
        cmake_tests = False

    git_root = oshelpers.fixpath(githelpers.get_root())
    build_dir = oshelpers.fixpath(git_root, constants.build_root)
    print("Creating build dirs under '%s'" %build_dir)

    # Install NuGet dependencies
    install_nuget_package()

    # Pass version (if specified) to CMake
    product_version_cmake = ''
    if args.version:
        product_version_cmake = "-DPRODUCT_VERSION=" + args.version

    windows_win32_cmake = [generator_vs2019, "-A Win32", product_version_cmake]
    windows_x64_cmake = [generator_vs2019, "-A x64", product_version_cmake]
    windows_arm_cmake[:0] = [generator_vs2019, "-A ARM", product_version_cmake]
    windows_arm64_cmake[:0] = [generator_vs2019, "-A ARM64", product_version_cmake]

    if (cmake_windows):
        create_build_folder_for_platform_architecture(build_dir, "Windows", "Win32", windows_win32_cmake, cmake_tests, git_root)
        create_build_folder_for_platform_architecture(build_dir, "Windows", "x64", windows_x64_cmake, cmake_tests, git_root)
        create_build_folder_for_platform_architecture(build_dir, "Windows", "ARM", windows_arm_cmake, cmake_tests, git_root)
        create_build_folder_for_platform_architecture(build_dir, "Windows", "ARM64", windows_arm64_cmake, cmake_tests, git_root)
    if (cmake_windowsstore):
        windows_store_flags = ["-DCMAKE_SYSTEM_NAME=WindowsStore -DCMAKE_SYSTEM_VERSION=10.0"]
        windowsstore_win32_cmake = windows_win32_cmake + windows_store_flags
        windowsstore_x64_cmake = windows_x64_cmake + windows_store_flags
        windowsstore_arm_cmake = windows_arm_cmake + windows_store_flags
        windowsstore_arm64_cmake = windows_arm64_cmake + windows_store_flags
        create_build_folder_for_platform_architecture(build_dir, "WindowsStore", "Win32", windowsstore_win32_cmake, cmake_tests, git_root)
        create_build_folder_for_platform_architecture(build_dir, "WindowsStore", "x64", windowsstore_x64_cmake, cmake_tests, git_root)
        create_build_folder_for_platform_architecture(build_dir, "WindowsStore", "arm", windowsstore_arm_cmake, cmake_tests, git_root)
        create_build_folder_for_platform_architecture(build_dir, "WindowsStore", "arm64", windowsstore_arm64_cmake, cmake_tests, git_root)
    if (cmake_android):
        create_build_folder_for_platform_architecture(build_dir, "Android", "x86-Debug"  , android_x86_debug_cmake, cmake_tests, git_root)
        create_build_folder_for_platform_architecture(build_dir, "Android", "x86", android_x86_release_cmake, cmake_tests, git_root)
        create_build_folder_for_platform_architecture(build_dir, "Android", "armeabi-v7a-Debug", android_armeabi_v7a_debug_cmake, cmake_tests, git_root)
        create_build_folder_for_platform_architecture(build_dir, "Android", "armeabi-v7a", android_armeabi_v7a_release_cmake, cmake_tests, git_root)
        create_build_folder_for_platform_architecture(build_dir, "Android", "arm64-v8a-Debug", android_arm64_v8a_debug_cmake, cmake_tests, git_root)
        create_build_folder_for_platform_architecture(build_dir, "Android", "arm64-v8a", android_arm64_v8a_release_cmake, cmake_tests, git_root)


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
