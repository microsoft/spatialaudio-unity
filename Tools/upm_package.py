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
    parser.add_argument("-o", "--output", help="Output location, will use default build/npm location if unspecified", type=str.lower)
    parser.add_argument("-v", "--version", help="Semantic version string for the package", type=str.lower)
    parser.add_argument("-p", "--publish", help="Publish the package to NPM feed", action='store_true')
    parser.add_argument("-t", "--tarball", help="Publish specified tarball", type=str.lower)
    parser.add_argument("-d", "--dryrun", help="Simulate publishing without pushing", action='store_true')
    args = parser.parse_args()

    if not args.tarball:
        # Copy plugin binaries to project location
        if not args.stage:
            stage.stage_binaries()
        else:
            stage.stage_binaries(args.stage)

        git_root = oshelpers.fixpath(githelpers.get_root())

        # Default output path is under build/unity
        npm_package_location = oshelpers.fixpath(git_root, constants.build_root, "npm")
        if args.output:
            npm_package_location = args.output

        if not os.path.isdir(npm_package_location):
            os.mkdir(npm_package_location)

        unity_project_full_path = oshelpers.fixpath(git_root, constants.unity_project_dir, "Assets", constants.spatializer_plugin_name)
        # Specify the package version before packing
        result = subprocess.run(["cmd", "/c", "npm version", args.version, "--allow-same-version"], cwd=unity_project_full_path)
        local_copy = False
        if args.publish:
            npm_command = ["cmd", "/c", "npm publish"]
            if args.dryrun:
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
                npm_package_full_path = oshelpers.fixpath(unity_project_full_path, constants.spatializer_npm_package_name + "-" + args.version + ".tgz")
                shutil.move(npm_package_full_path, npm_package_location)
                print("Package successfully generated: " + npm_package_full_path)
                for file in os.listdir(npm_package_location):
                    if file.endswith(".tgz"):
                        print("Package successfully moved to " + oshelpers.fixpath(npm_package_location, file))
            else:
                print("Package successfully published")
    else:
        if not os.path.exists(args.tarball):
            print("Can't find tarball " + args.tarball)
            exit
        npm_command = ["cmd", "/c", "npm publish", args.tarball]
        if args.dryrun:
            npm_command.append("--dry-run")
            
        print(npm_command)
        result = subprocess.run(npm_command)
        if (result.returncode != 0):
            print("Package generation failed!")
            print(result.stdout)
            print(result.stderr)
        else:
            print("Published tarball!")

if __name__ == '__main__':
    main()