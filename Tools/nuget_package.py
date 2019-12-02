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
import stage
import subprocess

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-s", "--stage", help="Copies built spatializer binaries from passed artifacts directory to Unity project locations", type=str.lower)
    parser.add_argument("-o", "--output", help="Output location, will use default build/unity location if unspecified", type=str.lower)
    parser.add_argument("-v", "--version", help="Version number", type=str.lower)
    args = parser.parse_args()

    # Copy plugin binaries to project location
    if not args.stage:
        stage.stage_binaries()
    else:
        stage.stage_binaries(args.stage)

    git_root = githelpers.get_root()

    # Default output path is under build/unity
    nuget_package_location = oshelpers.fixpath(git_root, constants.build_root, "nuget")
    if args.output:
        nuget_package_location = args.output

    if not os.path.isdir(nuget_package_location):
        os.mkdir(nuget_package_location)

    unity_project_full_path = oshelpers.fixpath(git_root, constants.unity_project_dir)
    nuspec_path = oshelpers.fixpath(unity_project_full_path, "Assets", constants.spatializer_plugin_name, constants.spatializer_plugin_name + ".nuspec")
    print(nuspec_path)
    print(nuget_package_location)
    print(args.version)
    nuget_package_creation_command = "nuget pack " + nuspec_path + " -outputdir " + nuget_package_location + " -exclude " + "*.nuspec.meta" + " -Version " + args.version
    print(nuget_package_creation_command)
    result = subprocess.run(nuget_package_creation_command)
    if (result.returncode != 0):
        print("Package generation failed!")
        print(result.stdout)
        print(result.stderr)


if __name__ == '__main__':
    main()