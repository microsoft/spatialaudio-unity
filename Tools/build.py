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

def call_msbuild(platform, architecture, configuration = "relwithdebinfo", clean = False):
    git_root = oshelpers.fixpath(githelpers.get_root())
    solution = oshelpers.fixpath(git_root, constants.build_root, platform, architecture, constants.solution_file)
    subprocess.run("msbuild -m " + solution + " /p:configuration=" + configuration)

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-p", "--platforms", help="Platforms to build e.g. windows, windowsstore", type=str.lower)
    parser.add_argument("-a", "--architectures", help="Architectures to build e.g. x86, x64, arm, arm64", type=str.lower)
    parser.add_argument("-c", "--configurations", help="Configurations to build e.g. debug, relwithdebinfo", type=str.lower)
    parser.add_argument("--clean", help="Clean and build", type=str.lower)
    args = parser.parse_args()

    build_platforms = constants.valid_platforms
    if args.platforms:
        build_platforms = args.platforms.split(",")
        for platform in build_platforms:
            if platform not in constants.valid_platforms:
                sys.exit("Invalid build platform " + platform)
    build_architectures = constants.valid_architectures
    if args.architectures:
        build_architectures = args.architectures.split(",")
        for arch in build_architectures:
            if arch not in constants.valid_architectures:
                sys.exit("Invalid build architecture " + arch)
    build_configurations = constants.valid_configurations
    if args.configurations:
        build_configurations = args.configurations.split(",")
        for config in build_configurations:
            if config not in constants.valid_configurations:
                sys.exit("Invalid build configuration " + config)
    clean = False
    if args.clean:
        clean = True

    # Build as specified
    for platform in build_platforms:
        for arch in build_architectures:
            for config in build_configurations:
                call_msbuild(platform, arch, config, clean)

if __name__ == '__main__':
    main()


