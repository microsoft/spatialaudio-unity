# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.
#!/usr/bin/env python3
import sys
# Prevent Python from generating .pyc files
sys.dont_write_bytecode = True

build_root = "build"
solution_file = "Microsoft.SpatialAudio.Unity.sln"
spatializer_dll = "AudioPluginMicrosoftSpatializer.dll"
spatializer_plugin_name = "Microsoft.SpatialAudio.Spatializer.Unity"
spatializer_npm_package_name= "com.microsoft.spatialaudio.spatializer.unity"
unity_project_dir = "Source/Plugins/IsacPluginGenerator"
valid_platforms = ["windows", "windowsstore"]
valid_architectures = ["x86", "x64", "arm", "arm64"]
valid_configurations = ["debug", "relwithdebinfo"]