# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.
#!/usr/bin/env python3
import sys
# Prevent Python from generating .pyc files
sys.dont_write_bytecode = True

import argparse
import constants
import githelpers
import os
import oshelpers
import shutil
import subprocess

def copy_file(source_file, target_dir):
    print("Copying " + source_file + " to " + target_dir)
    shutil.copy(source_file, target_dir)

def copy_spatializer(source_dir, target_dir):
    copy_file(os.path.join(source_dir, constants.spatializer_dll), target_dir)
    copy_file(os.path.join(source_dir, constants.spatializer_pdb), target_dir)

def stage_binaries(artifacts_path = ""):
    git_root = oshelpers.fixpath(githelpers.get_root())
    plugin_path_under_project = oshelpers.fixpath(os.path.join(constants.unity_project_dir, "Assets", constants.spatializer_plugin_name, "Plugins"))

    if not artifacts_path:
        source_dir_x64_desktop = oshelpers.fixpath(os.path.join(git_root, constants.build_root, "windows", "x64", "bin", "relwithdebinfo"))
        source_dir_x86_desktop = oshelpers.fixpath(os.path.join(git_root, constants.build_root, "windows", "x86", "bin", "relwithdebinfo"))
        source_dir_x64_uwp = oshelpers.fixpath(os.path.join(git_root, constants.build_root, "windowsstore", "x64", "bin", "relwithdebinfo"))
        source_dir_x86_uwp = oshelpers.fixpath(os.path.join(git_root, constants.build_root, "windowsstore", "x86", "bin", "relwithdebinfo"))
        source_dir_ARM_uwp = oshelpers.fixpath(os.path.join(git_root, constants.build_root, "windowsstore", "arm", "bin", "relwithdebinfo"))
        source_dir_ARM64_uwp = oshelpers.fixpath(os.path.join(git_root, constants.build_root, "windowsstore", "arm64", "bin", "relwithdebinfo"))
    else:
        source_dir_x64_desktop = oshelpers.fixpath(os.path.join(artifacts_path, "windows_x64_desktop"))
        source_dir_x86_desktop = oshelpers.fixpath(os.path.join(artifacts_path, "windows_x86_desktop"))
        source_dir_x64_uwp = oshelpers.fixpath(os.path.join(artifacts_path, "windows_x64_uwp"))
        source_dir_x86_uwp = oshelpers.fixpath(os.path.join(artifacts_path, "windows_x86_uwp"))
        source_dir_ARM_uwp = oshelpers.fixpath(os.path.join(artifacts_path, "windows_arm_uwp"))
        source_dir_ARM64_uwp = oshelpers.fixpath(os.path.join(artifacts_path, "windows_arm64_uwp"))

    target_path_x64_desktop = oshelpers.fixpath(os.path.join(git_root, plugin_path_under_project, "x86_64"))
    target_path_x86_desktop = oshelpers.fixpath(os.path.join(git_root, plugin_path_under_project, "x86"))
    target_path_x64_uwp = oshelpers.fixpath(os.path.join(git_root, plugin_path_under_project, "WSA", "x86_64"))
    target_path_x86_uwp = oshelpers.fixpath(os.path.join(git_root, plugin_path_under_project, "WSA", "x86"))
    target_path_ARM_uwp = oshelpers.fixpath(os.path.join(git_root, plugin_path_under_project, "WSA", "arm"))
    target_path_ARM64_uwp = oshelpers.fixpath(os.path.join(git_root, plugin_path_under_project, "WSA", "arm64"))

    copy_spatializer(source_dir_x64_desktop, target_path_x64_desktop)
    copy_spatializer(source_dir_x86_desktop, target_path_x86_desktop)
    copy_spatializer(source_dir_x64_uwp, target_path_x64_uwp)
    copy_spatializer(source_dir_x86_uwp, target_path_x86_uwp)
    copy_spatializer(source_dir_ARM_uwp, target_path_ARM_uwp)
    copy_spatializer(source_dir_ARM64_uwp, target_path_ARM64_uwp)