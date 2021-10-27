using System.Collections.Generic;
using System.IO;
using System.Runtime.InteropServices;
using UnityEditor;
using UnityEngine;

namespace Microsoft.SpatialAudio.Spatializer
{
    public enum SpatializationTarget
    {
        Headphones = 0,
        Speakers = 1,
    }

    public class SpatializerSettings : ScriptableObject
    {
        public const string _SpatializerSettingsAssetsPath = @"Assets\Microsoft.SpatialAudio.Spatializer.Unity\Scripts\SpatializerSettings.asset";

        [SerializeField]
        // Use headphones as default
        private SpatializationTarget _SpatializationTarget = SpatializationTarget.Headphones;

        internal static SpatializerSettings GetOrCreateSettings()
        {
            var settings = AssetDatabase.LoadAssetAtPath<SpatializerSettings>(_SpatializerSettingsAssetsPath);
            if (settings == null)
            {
                settings = ScriptableObject.CreateInstance<SpatializerSettings>();
                settings._SpatializationTarget = SpatializationTarget.Headphones;
                AssetDatabase.CreateAsset(settings, _SpatializerSettingsAssetsPath);
                AssetDatabase.SaveAssets();
                Spatializer_SetSpatializationTarget((int)settings._SpatializationTarget);
            }
            return settings;
        }
        public static SerializedObject GetSerializedSettings()
        {
            return new SerializedObject(GetOrCreateSettings());
        }

        [DllImport("AudioPluginMicrosoftSpatializerCrossPlatform")]
        private static extern bool Spatializer_SetSpatializationTarget(int target);
    }
}
