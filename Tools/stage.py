# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.
#!/usr/bin/env python3
import sys
# Prevent Python from generating .pyc files
sys.dont_write_bytecode = True

import constants
import githelpers
import oshelpers
import shutil

def copy_file(source_file, target_dir):
    print("Copying " + source_file + " to " + target_dir)
    shutil.copy(source_file, target_dir)

def copy_spatializer(spatializer_name, platform, source_dir, target_dir):
    if (platform == "Windows" or platform == "WindowsStore"):
        copy_file(oshelpers.fixpath(source_dir, spatializer_name + ".dll"), target_dir)
        copy_file(oshelpers.fixpath(source_dir, spatializer_name + ".pdb"), target_dir)
    else:
        copy_file(oshelpers.fixpath(source_dir, "lib" + spatializer_name + ".so"), target_dir)

def copy_external(name, platform, arch, target_path):
    git_root = oshelpers.fixpath(githelpers.get_root())
    platform_arch_path = oshelpers.fixpath(git_root, "Source", "External", name, platform, arch)
    libname = name + ".dll"
    pdbname = name + ".pdb"
    if (platform == "Android"):
        libname = "lib" + name + ".so"
        pdbname = ""
    libpath = oshelpers.fixpath(platform_arch_path, libname)
    copy_file(libpath, target_path)
    if (pdbname):
        pdbpath = oshelpers.fixpath(platform_arch_path, pdbname)
        copy_file(pdbpath, target_path)

def stage_binaries_isac(artifacts_path = ""):
    git_root = oshelpers.fixpath(githelpers.get_root())
    plugin_path_under_project = oshelpers.fixpath(constants.isac_unity_project_dir, "Assets", constants.isac_spatializer_plugin_name, "Plugins")

    if not artifacts_path:
        source_dir_x64_desktop = oshelpers.fixpath(git_root, constants.build_root, "windows", "x64", "bin", "relwithdebinfo")
        source_dir_x86_desktop = oshelpers.fixpath(git_root, constants.build_root, "windows", "Win32", "bin", "relwithdebinfo")
        source_dir_x64_uwp = oshelpers.fixpath(git_root, constants.build_root, "windowsstore", "x64", "bin", "relwithdebinfo")
        source_dir_x86_uwp = oshelpers.fixpath(git_root, constants.build_root, "windowsstore", "Win32", "bin", "relwithdebinfo")
        source_dir_ARM_uwp = oshelpers.fixpath(git_root, constants.build_root, "windowsstore", "ARM", "bin", "relwithdebinfo")
        source_dir_ARM64_uwp = oshelpers.fixpath(git_root, constants.build_root, "windowsstore", "ARM64", "bin", "relwithdebinfo")
    else:
        source_dir_x64_desktop = oshelpers.fixpath(artifacts_path, "windows_x64_desktop")
        source_dir_x86_desktop = oshelpers.fixpath(artifacts_path, "windows_x86_desktop")
        source_dir_x64_uwp = oshelpers.fixpath(artifacts_path, "windows_x64_uwp")
        source_dir_x86_uwp = oshelpers.fixpath(artifacts_path, "windows_x86_uwp")
        source_dir_ARM_uwp = oshelpers.fixpath(artifacts_path, "windows_arm_uwp")
        source_dir_ARM64_uwp = oshelpers.fixpath(artifacts_path, "windows_arm64_uwp")

    target_path_x64_desktop = oshelpers.fixpath(git_root, plugin_path_under_project, "x86_64")
    target_path_x86_desktop = oshelpers.fixpath(git_root, plugin_path_under_project, "x86")
    target_path_x64_uwp = oshelpers.fixpath(git_root, plugin_path_under_project, "WSA", "x86_64")
    target_path_x86_uwp = oshelpers.fixpath(git_root, plugin_path_under_project, "WSA", "x86")
    target_path_ARM_uwp = oshelpers.fixpath(git_root, plugin_path_under_project, "WSA", "arm")
    target_path_ARM64_uwp = oshelpers.fixpath(git_root, plugin_path_under_project, "WSA", "arm64")

    copy_spatializer(constants.isac_spatializer_name, "Windows", source_dir_x64_desktop, target_path_x64_desktop)
    copy_spatializer(constants.isac_spatializer_name, "Windows", source_dir_x86_desktop, target_path_x86_desktop)
    copy_spatializer(constants.isac_spatializer_name, "WindowsStore", source_dir_x64_uwp, target_path_x64_uwp)
    copy_spatializer(constants.isac_spatializer_name, "WindowsStore", source_dir_x86_uwp, target_path_x86_uwp)
    copy_spatializer(constants.isac_spatializer_name, "WindowsStore", source_dir_ARM_uwp, target_path_ARM_uwp)
    copy_spatializer(constants.isac_spatializer_name, "WindowsStore", source_dir_ARM64_uwp, target_path_ARM64_uwp)


def stage_binaries_crossplatform(artifacts_path = ""):
    git_root = oshelpers.fixpath(githelpers.get_root())
    plugin_path_under_project = oshelpers.fixpath(constants.crossplatform_unity_project_dir, "Assets", constants.crossplatform_spatializer_plugin_name, "Plugins")

    if not artifacts_path:
        source_dir_x64_desktop = oshelpers.fixpath(git_root, constants.build_root, "windows", "x64", "bin", "relwithdebinfo")
        source_dir_x86_desktop = oshelpers.fixpath(git_root, constants.build_root, "windows", "Win32", "bin", "relwithdebinfo")
        source_dir_x64_uwp = oshelpers.fixpath(git_root, constants.build_root, "windowsstore", "x64", "bin", "relwithdebinfo")
        source_dir_x86_uwp = oshelpers.fixpath(git_root, constants.build_root, "windowsstore", "Win32", "bin", "relwithdebinfo")
        source_dir_ARM_uwp = oshelpers.fixpath(git_root, constants.build_root, "windowsstore", "ARM", "bin", "relwithdebinfo")
        source_dir_ARM64_uwp = oshelpers.fixpath(git_root, constants.build_root, "windowsstore", "ARM64", "bin", "relwithdebinfo")
        source_dir_ARM_android = oshelpers.fixpath(git_root, constants.build_root, "Android", "armeabi-v7a", "lib", "relwithdebinfo")
        source_dir_ARM64_android = oshelpers.fixpath(git_root, constants.build_root, "Android", "arm64-v8a", "lib", "relwithdebinfo")
    else:
        source_dir_x64_desktop = oshelpers.fixpath(artifacts_path, "windows_x64_desktop")
        source_dir_x86_desktop = oshelpers.fixpath(artifacts_path, "windows_x86_desktop")
        source_dir_x64_uwp = oshelpers.fixpath(artifacts_path, "windows_x64_uwp")
        source_dir_x86_uwp = oshelpers.fixpath(artifacts_path, "windows_x86_uwp")
        source_dir_ARM_uwp = oshelpers.fixpath(artifacts_path, "windows_arm_uwp")
        source_dir_ARM64_uwp = oshelpers.fixpath(artifacts_path, "windows_arm64_uwp")
        source_dir_ARM_android = oshelpers.fixpath(artifacts_path, "android_arm")
        source_dir_ARM64_android = oshelpers.fixpath(artifacts_path, "android_arm64")

    target_path_x64_desktop = oshelpers.fixpath(git_root, plugin_path_under_project, "x86_64")
    target_path_x86_desktop = oshelpers.fixpath(git_root, plugin_path_under_project, "x86")
    target_path_x64_uwp = oshelpers.fixpath(git_root, plugin_path_under_project, "WSA", "x86_64")
    target_path_x86_uwp = oshelpers.fixpath(git_root, plugin_path_under_project, "WSA", "x86")
    target_path_ARM_uwp = oshelpers.fixpath(git_root, plugin_path_under_project, "WSA", "arm")
    target_path_ARM64_uwp = oshelpers.fixpath(git_root, plugin_path_under_project, "WSA", "arm64")
    target_path_ARM_android = oshelpers.fixpath(git_root, plugin_path_under_project, "Android", "libs", "armeabi-v7a")
    target_path_ARM64_android = oshelpers.fixpath(git_root, plugin_path_under_project, "Android", "libs", "arm64-v8a")

    copy_spatializer(constants.crossplatform_spatializer_name, "Windows", source_dir_x64_desktop, target_path_x64_desktop)
    copy_spatializer(constants.crossplatform_spatializer_name, "Windows", source_dir_x86_desktop, target_path_x86_desktop)
    copy_spatializer(constants.crossplatform_spatializer_name, "WindowsStore", source_dir_x64_uwp, target_path_x64_uwp)
    copy_spatializer(constants.crossplatform_spatializer_name, "WindowsStore", source_dir_x86_uwp, target_path_x86_uwp)
    copy_spatializer(constants.crossplatform_spatializer_name, "WindowsStore", source_dir_ARM_uwp, target_path_ARM_uwp)
    copy_spatializer(constants.crossplatform_spatializer_name, "WindowsStore", source_dir_ARM64_uwp, target_path_ARM64_uwp)
    copy_spatializer(constants.crossplatform_spatializer_name, "Android", source_dir_ARM_android, target_path_ARM_android)
    copy_spatializer(constants.crossplatform_spatializer_name, "Android", source_dir_ARM64_android, target_path_ARM64_android)

    copy_external("HrtfDsp", "Windows", "x64", target_path_x64_desktop)
    copy_external("HrtfDsp", "Windows", "Win32", target_path_x86_desktop)
    copy_external("HrtfDsp", "WindowsStore", "Win32", target_path_x86_uwp)
    copy_external("HrtfDsp", "WindowsStore", "x64", target_path_x64_uwp)
    copy_external("HrtfDsp", "WindowsStore", "ARM", target_path_ARM_uwp)
    copy_external("HrtfDsp", "WindowsStore", "ARM64", target_path_ARM64_uwp)
    copy_external("HrtfDsp", "Android", "armeabi-v7a", target_path_ARM_android)
    copy_external("HrtfDsp", "Android", "arm64-v8a", target_path_ARM64_android)
