# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.
#!/usr/bin/env python3
import githelpers
import os
import oshelpers
import sys
# Prevent Python from generating .pyc files
sys.dont_write_bytecode = True

ndk_root = os.getenv("ANDROID_NDK_HOME","")
android_toolchain = oshelpers.fixpath(ndk_root, "build", "cmake", "android.toolchain.cmake")
android_make = oshelpers.fixpath(ndk_root,"prebuilt", "windows-x86_64", "bin", "make.exe")

build_root = "build"
externals_path = oshelpers.fixpath(githelpers.get_root(), "Source", "External")
solution_file = "Microsoft.SpatialAudio.Unity.sln"

# CrossPlatform spatializer with Windows and Android support
crossplatform_spatializer_name = "AudioPluginMicrosoftSpatializerCrossPlatform"
crossplatform_spatializer_npm_package_name= "com.microsoft.spatialaudio.spatializer.unity"
crossplatform_spatializer_plugin_name = "Microsoft.SpatialAudio.Spatializer.Unity"
crossplatform_unity_project_dir = "Source/Plugins/CrossPlatformPluginGenerator"

# Where to get the Universal Package dependencies
aipmr_azure_org = "https://dev.azure.com/aipmr/"
aipmr_package_feed = "SpatialAudio-packages"
hrtfdsp_package_name = "pa-hrtfdsp"
hrtfdsp_package_version = "2.1.727-prerelease"

# Supported build platforms and configs
build_platform_arch_map = {
    "Windows": ["Win32", "x64"],
    "WindowsStore": ["Win32", "x64", "ARM", "ARM64"],
    "Android": ["x86", "armeabi-v7a", "arm64-v8a"]
}
unity_platform_arch_map = {
    "Windows": {"x86":"x86", "x64":"x86_x64"},
    "WindowsStore": {"x86":"x86", "x64":"x86_x64", "ARM":"ARM", "ARM64":"ARM64"},
    "Android": {"x86":"x86", "armeabi-v7a":"armeabi-v7a", "arm64-v8a":"arm64-v8a"}
}
valid_configurations = ["debug", "relwithdebinfo"]