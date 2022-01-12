# How to add the plugin to Unity projects

## 1. Add the spatializer package reference to manifest.json
This is the recommended way to add the plugin package to a project. 
- Open the `manifest.json` file from the project in a text editor
- Add Microsoft's scoped registry and the package reference  
    <pre>
    {
        "scopedRegistries": [
        {
            "name": "Microsoft Spatializer",
            "url": "https://microsoft.pkgs.visualstudio.com/Analog/_packaging/MixedReality-UPM-Internal/npm/registry/",
            "scopes": [
            "com.microsoft.spatialaudio"
            ]
        },
        ...
        ],
        "dependencies": {
            ...
            "com.microsoft.spatialaudio.spatializer.unity": "2.0.5-prerelease",
            ...
        }
    }
    </pre>

For an example, see the [sample project manifest.](../samples/MicrosoftSpatializerSample/Packages/manifest.json)

The plugin package can also be downloaded from GitHub releases and imported into the project using `Assets > Import Package > Custom Package` menu.  

## 2. Configure the project to use Microsoft Spatializer
- Open the project settings using `Edit > Project Settings`
- In the `Audio` tab, select `Microsoft Spatializer` in the `Spatializer Settings` dropdown. No other changes are necessary.

    ![Project Settings](./Images/ProjectSettings.png)

## 3. Add the Microsoft Spatializer Mixer to the project
`Microsoft Spatializer Mixer` effect is necessary to process all the spatialized audio objects in a scene.
- Add an audio mixer to the project
  - In the `Project` pane, right-click on the `Assets` folder, then `Create > Audio Mixer`
  - Give the mixer a name, e.g. `Master` 
  - Open the mixer and right-click, then select `Add effect at bottom > Microsoft Spatializer Mixer`

    ![Microsoft Spatializer Mixer](./Images/SpatializerMixerSetup.png)

## 4. Spatialize audio objects
- Setup spatialized audio objects as usual, making sure to route the `Output` to the `Master` audio mixer setup above.

    ![Audio Object Settings](./Images/AudioObjectSettings.png)



