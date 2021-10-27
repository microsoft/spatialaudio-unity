using System.Collections.Generic;
using System.IO;
using UnityEditor;
using UnityEngine;
using UnityEngine.UIElements;

namespace Microsoft.SpatialAudio.Spatializer.Editor
{
    // Create SpatializerSettingsProvider by deriving from SettingsProvider:
    class SpatializerSettingsProvider : SettingsProvider
    {
        private SerializedObject _SpatializerSettings;

        class Styles
        {
            public static GUIContent SpatializatiionTarget = new GUIContent("Spatialization Target");
        }

        public SpatializerSettingsProvider(string path, SettingsScope scope = SettingsScope.User)
            : base(path, scope) 
        {
            label = "Microsoft Spatializer Settings";
            // Populate the search keywords to enable smart search filtering and label highlighting:
            keywords = new HashSet<string>(new[] { "Microsoft", "Spatializer", "HRTF" });
        }

        public static bool IsSettingsAvailable()
        {
            return File.Exists(SpatializerSettings._SpatializerSettingsAssetsPath);
        }

        public override void OnActivate(string searchContext, VisualElement rootElement)
        {
            // This function is called when the user clicks on the Spatializer element in the Settings window.
            _SpatializerSettings = SpatializerSettings.GetSerializedSettings();
        }

        public override void OnGUI(string searchContext)
        {
            _SpatializerSettings.Update();

            var spatializationTarget = (SpatializationTarget)_SpatializerSettings.FindProperty("_SpatializationTarget").intValue;
            EditorGUILayout.PropertyField(_SpatializerSettings.FindProperty("_SpatializationTarget"), Styles.SpatializatiionTarget);
        }

        // Register the SettingsProvider
        [SettingsProvider]
        public static SettingsProvider CreateSpatializerSettingsProvider()
        {
            if (IsSettingsAvailable())
            {
                var provider = new SpatializerSettingsProvider("Project/SpatializerSettingsProvider", SettingsScope.Project);

                // Automatically extract all keywords from the Styles.
                provider.keywords = GetSearchKeywordsFromGUIContentProperties<Styles>();
                return provider;
            }

            // Settings Asset doesn't exist yet; no need to display anything in the Settings window.
            return null;
        }
    }
}