// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

using UnityEngine;
using UnityEditor;

namespace Microsoft.SpatialAudio.Spatializer
{
    [DisallowMultipleComponent]
    [RequireComponent(typeof(AudioSource))]
    public class RoomEffectSendLevel : MonoBehaviour
    {
        private const float c_RoomEffectSendPowerMin = -100;
        private const float c_RoomEffectSendPowerMax = 20;
        private const float c_RoomEffectSendPowerMaxDistance = 100;

        private const string c_RoomEffectSendPowerLabel = "Room Effect Send Level (dB)";

        // Spatializer parameter indices
        public enum SpatializerParams
        {
            RoomEffectSendPower = 0,
            Count
        }

        [CurveDimensions(0, c_RoomEffectSendPowerMaxDistance, c_RoomEffectSendPowerMin, c_RoomEffectSendPowerMax, c_RoomEffectSendPowerLabel)]
        [Tooltip("Calculate the amount of non-spatialized source signal that should be processed for the room effect (typically reverb) in dB based on the source-listener distance. Default=0dB, Range [-100dB,20dB]")]
        [ContextMenuItem("Reset", "ResetRoomEffectSendPowerCurve")]
        public AnimationCurve RoomEffectSendPowerCurve = AnimationCurve.Linear(0, 0, c_RoomEffectSendPowerMaxDistance, 0);

        private float _LastRoomEffectSendPower = float.MinValue;

        private void ResetRoomEffectSendPowerCurve()
        {
            RoomEffectSendPowerCurve = AnimationCurve.Linear(0, 0, c_RoomEffectSendPowerMaxDistance, 0);

#if UNITY_EDITOR
            // This forces the inspector preview of the animation curve to refresh
            AssetDatabase.Refresh();
#endif
        }

        void Update()
        {
            var source = GetComponent<AudioSource>();

            // Source-listener distance
            float distance = Vector3.Distance(transform.position, Camera.main.transform.position);

            // Get the room effect send level for this distance
            float roomEffectSendPower = RoomEffectSendPowerCurve.Evaluate(distance);

            if (_LastRoomEffectSendPower != roomEffectSendPower)
            {
                _LastRoomEffectSendPower = roomEffectSendPower;
                source.SetSpatializerFloat((int)SpatializerParams.RoomEffectSendPower, _LastRoomEffectSendPower);
            }
        }
    }

#if UNITY_EDITOR
    /// <summary>
    /// Custom drawer for the startup of AnimationCurves. Sets the grid range for the curve editor.
    /// </summary>
    [CustomPropertyDrawer(typeof(CurveDimensions))]
    public class CurveDrawer : PropertyDrawer
    {
        public override void OnGUI(Rect position, SerializedProperty property, GUIContent label)
        {
            var curve = attribute as CurveDimensions;
            if (property.propertyType == SerializedPropertyType.AnimationCurve)
            {
                var rect = new Rect(curve.XMin, curve.YMin, System.Math.Abs(curve.XMin - curve.XMax), System.Math.Abs(curve.YMin - curve.YMax));
                EditorGUI.CurveField(position, property, Color.green, rect, new GUIContent(curve.Label));
            }
        }
    }
#endif

    /// <summary>
    /// Property attribute for setting the CurveField grid range.
    /// </summary>
    public class CurveDimensions : PropertyAttribute
    {
        public float XMin, XMax, YMin, YMax;
        public string Label;

        public CurveDimensions(float xMin, float xMax, float yMin, float yMax, string label)
        {
            this.XMin = xMin;
            this.XMax = xMax;
            this.YMin = yMin;
            this.YMax = yMax;
            this.Label = label;
        }
    }
}
