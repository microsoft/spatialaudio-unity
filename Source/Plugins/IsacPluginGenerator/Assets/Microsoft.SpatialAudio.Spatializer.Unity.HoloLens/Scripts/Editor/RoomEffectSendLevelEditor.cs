// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

using UnityEditor;

namespace Microsoft.SpatialAudio.Spatializer.Editor
{
    [CustomEditor(typeof(RoomEffectSendLevel))]
    [CanEditMultipleObjects]
    public class RoomEffectSendLevelEditor : UnityEditor.Editor
    {
        private SerializedProperty RoomEffectSendPowerCurve;

        public void OnEnable()
        {
            RoomEffectSendPowerCurve = serializedObject.FindProperty("RoomEffectSendPowerCurve");
        }

        public override void OnInspectorGUI()
        {
            serializedObject.UpdateIfRequiredOrScript();
            EditorGUILayout.PropertyField(RoomEffectSendPowerCurve);
            serializedObject.ApplyModifiedProperties();
        }
    }
}
