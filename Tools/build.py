# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.
#!/usr/bin/env python3
import sys
# Prevent Python from generating .pyc files
sys.dont_write_bytecode = True

import argparse
import constants
import subprocess
import oshelpers
import githelpers

def build(platform, architecture, configuration = "relwithdebinfo", clean = False):
    git_root = oshelpers.fixpath(githelpers.get_root())
    build_dir = oshelpers.fixpath(git_root, "build", platform, architecture)
    if (platform == "Windows" or platform == "WindowsStore"):
        solution = oshelpers.fixpath(git_root, constants.build_root, platform, architecture, constants.solution_file)
        subprocess.run("msbuild -m " + solution + " /p:configuration=" + configuration)
    elif (platform == "Android"):
        androidClean = constants.android_make + " clean"
        androidArchSuffix = "-Debug" if configuration == "Debug" else ""
        subprocess.run(constants.android_make, cwd = build_dir + androidArchSuffix, check = True)

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--platform", help="Platform to build")
    parser.add_argument("--clean", help="Clean and build", type=str.lower)
    args = parser.parse_args()

    clean = False
    if args.clean:
        clean = True

    # Build as specified
    for platform in constants.build_platform_arch_map.keys():
        platform_architectures = constants.build_platform_arch_map[platform]
        for arch in platform_architectures:
            for config in constants.valid_configurations:
                if (args.platform == None or args.platform == platform):
                    build(platform, arch, config, clean)

if __name__ == '__main__':
    main()


