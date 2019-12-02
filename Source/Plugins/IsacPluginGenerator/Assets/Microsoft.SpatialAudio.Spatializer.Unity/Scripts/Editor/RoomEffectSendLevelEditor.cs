// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
using UnityEditor;
using UnityEngine;

[CustomEditor(typeof(RoomEffectSendLevel))]
[CanEditMultipleObjects]
public class RoomEffectSendLevelEditor : Editor
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
