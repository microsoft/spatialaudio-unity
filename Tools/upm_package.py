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

def create_npm_package(project_dir, plugin_name, package_name, package_location, version, publish=False, dry_run=True):
    git_root = oshelpers.fixpath(githelpers.get_root())
    unity_project_full_path = oshelpers.fixpath(git_root, project_dir, "Assets", plugin_name)
    # Specify the package version before packing
    result = subprocess.run(["cmd", "/c", "npm version", version, "--allow-same-version"], cwd=unity_project_full_path)
    local_copy = False
    if publish:
        npm_command = ["cmd", "/c", "npm publish"]
        if dry_run:
            npm_command.append("--dry-run")                
    else:
        local_copy = True
        npm_command = ["cmd", "/c", "npm pack"]
    print(npm_command)
    result = subprocess.run(npm_command, cwd=unity_project_full_path)
    if (result.returncode != 0):
        print("Package generation failed!")
        print(result.stdout)
        print(result.stderr)
    else:
        if local_copy:
            npm_package_full_path = oshelpers.fixpath(unity_project_full_path, package_name + "-" + version + ".tgz")
            shutil.move(npm_package_full_path, package_location)
            print("Package successfully generated: " + npm_package_full_path)
            for file in os.listdir(package_location):
                if file.endswith(".tgz"):
                    print("Package successfully moved to " + oshelpers.fixpath(package_location, file))
        else:
            print("Package successfully published")

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-s", "--stage", help="Copies built spatializer binaries from passed artifacts directory to Unity project locations", type=str.lower)
    parser.add_argument("-o", "--output", help="Output location, will use default build/npm location if unspecified", type=str.lower)
    parser.add_argument("-v", "--version", help="Semantic version string for the package", type=str.lower)
    parser.add_argument("-p", "--publish", help="Publish the package to NPM feed", action='store_true')
    parser.add_argument("-d", "--dryrun", help="Simulate publishing without pushing", action='store_true')
    args = parser.parse_args()

    # Copy plugin binaries to project location
    stage.stage_binaries_isac(args.stage)
    stage.stage_binaries_crossplatform(args.stage)

    git_root = oshelpers.fixpath(githelpers.get_root())

    # Default output path is under build/unity
    npm_package_location = oshelpers.fixpath(git_root, constants.build_root, "npm")
    if args.output:
        npm_package_location = args.output

    if not os.path.isdir(npm_package_location):
        os.mkdir(npm_package_location)

    create_npm_package(constants.crossplatform_unity_project_dir, constants.crossplatform_spatializer_plugin_name, constants.crossplatform_spatializer_npm_package_name, npm_package_location, args.version, args.publish, args.dryrun)
    create_npm_package(constants.isac_unity_project_dir, constants.isac_spatializer_plugin_name, constants.isac_spatializer_npm_package_name, npm_package_location, args.version, args.publish, args.dryrun)

if __name__ == '__main__':
    main()