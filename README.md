[![Licensed under the MIT License](https://img.shields.io/badge/License-MIT-blue.svg)](https://github.com/microsoft/spatialaudio-unity/blob/master/LICENSE)

# Introduction 
This repository provides plugins and tools for integrating spatial audio into your Unity 3D applications and games  
- A **cross-platform spatializer plugin (supported on Windows and Android)** that uses highly efficient spatial audio DSP processing algorithms.
- A sample Unity application that demonstrates cross-platform spatializer plugin configuration and usage.

# Getting started with Spatial Audio for Unity
Cloning this repository is not required to start using the Microsoft Spatializer in your Unity project. Visit the [documentation](https://docs.microsoft.com/en-us/windows/mixed-reality/spatial-sound-in-unity) for instructions on integrating the Microsoft Spatializer into your Unity project. For a more in-depth exploration of spatial audio, check out the [learning module](https://docs.microsoft.com/en-us/windows/mixed-reality/unity-spatial-audio-ch1). If you'd like to build the plugin yourself, see below.

### Choosing the right spatializer 
With requirements and features evolving over time, there are now 3 different Unity spatializer plugins available from Microsoft. Here's a brief description of their differences which can help decide the right plugin for a project.

#### [Microsoft Spatializer v2](https://github.com/microsoft/spatialaudio-unity/releases/tag/v2.0.30-prerelease)
This is the latest highly optimized cross-platform spatializer plugin for Windows and Android built from this repository. Although this plugin is currenly in the pre-release phase, it's being actively developed and recommended for any new projects, especially those that need to support both Windows and Android. This plugin uses the latest DSP engine that is highly optimized for both memory and CPU and fits well into Unity's audio engine architecture.

#### [Microsoft Spatializer v1](https://github.com/microsoft/spatialaudio-unity/releases/tag/v1.0.246)
While it is recommended to switch over to the latest cross-platform spatializer plugin, the previous [HoloLens 2 specific spatializer plugin version](https://github.com/microsoft/spatialaudio-unity/tree/v1.0.246) with hardware offload suppport, remains available on [GitHub releases](https://github.com/microsoft/spatialaudio-unity/releases/tag/v1.0.246) and on a UPM feed via [Mixed Reality Feature Tool](https://docs.microsoft.com/en-us/windows/mixed-reality/mrtk-unity/configuration/usingupm?view=mrtkunity-2021-05). This plugin can be useful for any *HoloLens 2 specific projects* where it can reduce the CPU usage by leveraging offloaded spatial audio DSP. This plugin utlizes Windows Spatial Audio Platform APIs that prevent the processed audio signal to flow back into Unity's audio engine which can make it cumbersome for supporting some audio design features, such as adding an environmental reverb to the spatial audio mix.    

#### Unity MS-HRTF Plugin
This is the original spatializer plugin which is shared for historical purposes. This plugin does not utilize the multi-source mixer plugin which leads to higer compute overhead than newer plugin offerings.

## Required Software

| ![Windows Logo](Documentation/Images/128px_Windows_logo.png)<br>[Windows SDK 18362+](https://developer.microsoft.com/en-US/windows/downloads/windows-10-sdk) | ![VS Logo](Documentation/Images/128px_Visual_Studio_2019.png)<br>[Visual Studio 2019](https://visualstudio.microsoft.com/vs/) | ![CMake Logo](Documentation/Images/128px_CMake_logo.png)<br>[CMake](https://cmake.org/) | ![Unity3D logo](Documentation/Images/128px_Official_unity_logo.png)<br>[Unity 2019+](https://unity.com/releases/2019-2?_ga=2.114950222.898171561.1571681098-1938809356.1563129846) | ![Python Logo](Documentation/Images/128pv_python_logo.png)<br>[Python 3+](https://www.python.org/downloads/) | ![NodeJS Logo](Documentation/Images/128px_NodeJs_Logo.png)<br>[Node.js](https://nodejs.org/en/download/) | ![Android Logo](Documentation/Images/Android_symbol_green_RGB.png)<br>[Android NDK](https://developer.android.com/ndk/downloads) 
| :---: | :---: | :---: | :---: | :---: | :---: | :---: |
| Windows 10 May 2019 Update SDK to build the spatializer plugin. | Visual Studio is used for code editing, deploying and building UWP app packages | CMake is required for generating Visual Studio 2019 projects | Unity 2019 is required to build the spatializer plugin package.<br>Plugin can be used on Unity 2018 LTS and higher versions. | Helper scripts for build and packaging use Python 3 and higher. | For UPM packaging. | Required for building Android binaries.

### Branch Guide
- This repository follows the [GitFlow branching model](https://nvie.com/posts/a-successful-git-branching-model/).
- Master branch is used for building release candidates and official releases. Direct pull requests into master are not allowed.
- Develop branch is used for staging ongoing work for the next official release and merged with master after extensive review and testing. Direct pull requests into develop branch are not allowed.
- Use feature branches to bring up individual features. Once a feature is ready and tested, use a pull request to merge it into the develop branch.

### Clone the Repository
`git clone https://github.com/microsoft/spatialaudio-unity.git --recurse-submodules`

If you forget to include submodules when cloning, add them with `git submodule update --init --recursive`

### Build Status
| Build | Branch | Status |
| :----:| :----: | :----: |
| Release | [master](https://github.com/microsoft/spatialaudio-unity/tree/master) | [![Release Build Status](https://dev.azure.com/microsoft/Analog/_apis/build/status/mixedreality/spatialaudio/unity/microsoft.spatialaudio-unity?branchName=master)](https://dev.azure.com/microsoft/Analog/_build/latest?definitionId=46637&branchName=master) |
| Validation | [develop](https://github.com/microsoft/spatialaudio-unity/tree/develop) | [![Validation Build Status](https://dev.azure.com/ms/spatialaudio-unity/_apis/build/status/microsoft.spatialaudio-unity?branchName=develop)](https://dev.azure.com/ms/spatialaudio-unity/_build/latest?definitionId=304&branchName=develop) |


### Local Build
- Launch "Developer Command Prompt for Visual Studio 2019".
- Switch directory to the root of your Git enlistment.
- Run the CMake script to generate Visual Studio 2019 projects:
  `python3 Tools\runcmake.py`
- Run the build script to build all flavors:
  `python3 Tools\build.py`
- To generate the Unity package:
  `python3 Tools\unity_package.py -u "c:\Program Files\Unity\Hub\Editor\2020.3.2f1\Editor" -v 2.0.0`
- To generate the UPM package:
  `python3 Tools\upm_package.py -v 2.0.0`

### Artifacts
- Build produces UPM and Unity asset packages
- Unity asset package is available under [releases tab](https://github.com/microsoft/spatialaudio-unity/releases)
- Unity asset packages are also available on a UPM feed via [Microsoft Mixed Reality Feature Tool](https://docs.microsoft.com/en-us/windows/mixed-reality/mrtk-unity/configuration/usingupm?view=mrtkunity-2021-05)


### Consuming the UPM Pacakge
- [`manifest.json`](Samples/MicrosoftSpatializerSample/Packages/manifest.json) in the sample project shows integration of UPM package into a project
  - Add/edit the `scopedRegistries` section to project's `manifest.json`
    ```
    "scopedRegistries": [
      {
        "name": "Microsoft Mixed Reality",
        "url": "https://pkgs.dev.azure.com/aipmr/MixedReality-Unity-Packages/_packaging/Unity-packages/npm/registry/",
        "scopes": [
          "com.microsoft.spatialaudio"
        ]
      }
    ],
    ```
  - And add the package name and version to the `dependencies` section
    ```
      "dependencies": {
      ...
      "com.microsoft.spatialaudio.spatializer.unity": "2.0.30-prerelease",
      ...
    }
    ```