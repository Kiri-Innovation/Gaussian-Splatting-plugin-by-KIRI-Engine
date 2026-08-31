#include <metal_stdlib>
using namespace metal;
#include "define.metal"
#include "struct.metal"
#include "bezierCurve.metal"
#include "geometry.metal"
#include "noise.metal"

kernel void compute_splat_animate(
    device float *position [[buffer(0)]],                   
    device BezierCurveInfo* bezierCurveInfos [[buffer(1)]],
    constant RenderInfo& renderInfo [[buffer(2)]],
    device float *viewPosition [[buffer(3)]], // out

    uint id [[thread_position_in_grid]]) 
{
    //values[id] = values[id] * 2.0f + 1.0f;

    if(id >= renderInfo.splatCount){
        return;
    }

    // input position is local position , output is view position
    int header = id*3;
    float4 positionLocal = float4(position[header + 0], position[header + 1], position[header + 2], 1.0f);
    float3 positionAnchor = (renderInfo.anchorModelMatrix * positionLocal).xyz;

    float3 anchorPoint = (renderInfo.anchorModelMatrix * float4(0.0, 0.0, 0.0, 1.0)).xyz;
    // compute animate here

    float3 displaceOffset =float3(0.0);
    float3 noiseOffset = float3(0.0);

    BezierCurveInfo u_colorRamp = bezierCurveInfos[0];
    BezierCurveInfo u_splatScaleRamp = bezierCurveInfos[1];
    BezierCurveInfo u_splatOpacityRamp = bezierCurveInfos[2];
    BezierCurveInfo u_splatDisplacementOffsetRamp = bezierCurveInfos[3];
    BezierCurveInfo u_splatDisplacementScaleRamp = bezierCurveInfos[4];
    BezierCurveInfo u_splatDisplacementRotationRamp = bezierCurveInfos[5];

   if (renderInfo.splatNoiseEnable == 1.0)
    {
        float noiseRadiusBound = max(10 * renderInfo.splatNoiseShapeSize, 0.0001);
        float3 noiseNormalDir = (positionAnchor - renderInfo.splatNoiseShapeCenter.xyz);
        noiseNormalDir /= renderInfo.splatNoiseShapeScaleXYZ.xyz * PERCENT;
        float noiseMask = step(length(noiseNormalDir), noiseRadiusBound);
    
        int octaves = renderInfo.splatNoiseOctaves;
        float persistence = renderInfo.splatNoisePersistence * PERCENT;
        float lacunarity = renderInfo.splatNoiseLacunarity * PERCENT;
        float3 p = positionAnchor;
        float3 noiseValue;
        float3 offset1 = float3(13.57, 47.13, 29.63) * 1000.0;
        float3 offset2 = float3(73.19, 57.21, 89.47) * 1000.0;
    
        noiseValue.x = renderInfo.splatNoiseStrengthX * PERCENT * FBM(p , octaves, persistence, lacunarity);
        noiseValue.y = renderInfo.splatNoiseStrengthY * PERCENT * FBM(p + offset1, octaves, persistence, lacunarity);
        noiseValue.z = renderInfo.splatNoiseStrengthZ * PERCENT * FBM(p + offset2, octaves, persistence, lacunarity);
        
        noiseOffset = noiseValue * 10 * renderInfo.splatNoiseStrength * noiseMask;
    }
    

    if (renderInfo.splatDisplacementEnable == 1.0)
    {
        float displaceRadiusBound = max(10 * renderInfo.splatDisplacementShapeSize, 0.0001);
        float3 displaceNormalDir = (positionAnchor - renderInfo.splatDisplacementShapeCenter.xyz);
        displaceNormalDir /= renderInfo.splatDisplacementShapeScaleXYZ.xyz * PERCENT;
        float displaceNormalLength = length(displaceNormalDir);
        float displaceMask = step(displaceNormalLength, displaceRadiusBound);
    
        float displaceLerpFactor = displaceNormalLength / displaceRadiusBound;
    
        float displaceOffsetRampFactor = 1.0 - GetValueFromBezierCurve(u_splatDisplacementOffsetRamp , displaceLerpFactor);
        float displaceScaleRampFactor = 1.0 - GetValueFromBezierCurve(u_splatDisplacementScaleRamp, displaceLerpFactor);
        float displaceRotationRampFactor = 1.0 - GetValueFromBezierCurve(u_splatDisplacementRotationRamp, displaceLerpFactor);
    
        float displaceScaleValue = mix(1.0, renderInfo.splatDisplacementScale * PERCENT, displaceScaleRampFactor);
        float3 displaceRotationValue = mix(float3(0.0), radians(renderInfo.splatDisplacementRotation.xyz), displaceRotationRampFactor);
        float3 displacePosition = displaceOffsetRampFactor * renderInfo.splatDisplacementOffset.xyz;
    
        float4x4 displaceModelMatrix = translateMatrix(displacePosition) *
                               eulerRotationXYZ(displaceRotationValue) *
                               scaleMatrix(float3(displaceScaleValue));

        float4 targetDisplacePoint = displaceModelMatrix * float4(positionAnchor, 1.0);
        displaceOffset = targetDisplacePoint.xyz - positionAnchor;
    //float3 displaceOffset = renderInfo.splatDisplacementShapeCenter.xyz * renderInfo.splatDisplacementShapeSize * displaceMask;
        displaceOffset *= displaceMask;
    }
   
    positionAnchor += noiseOffset + displaceOffset;

    if (renderInfo.splatInvertSphereEnable == 1.0)
    {
        float R = max(renderInfo.splatInvertSphereRaduis, 1e-6);
        float intensity = clamp(renderInfo.splatInvertSphereIntensity, 0.0, 1.0);
        float D = renderInfo.splatInvertSphereDistance;
        float mu = max(renderInfo.splatInvertSphereCompression, 0.0) * PERCENT;
        
        float3 sphereCenter = renderInfo.splatInvertSphereCenter.xyz;
        sphereCenter += anchorPoint;
        float3 offset = positionAnchor - sphereCenter;
        
        float r = length(offset);
        
        float3 direction = offset / r;
        float f_r = r ;
        
        if ( r > 1e-6) {
            float denom = 1.0 - (R / D);
        
            float v = ((R / r) - (R / D)) / denom;

            v = clamp(v, 0.0, 1.0);
            float h = 0.0;

            if (abs(mu) < 1e-6) {
                h = v;
            } else {
                h = log(1.0 + mu * v) / log(1.0 + mu);
            }
            
            float f_r_raw = mix(r, R * h, intensity);  
            f_r = f_r_raw;

        }
        positionAnchor = sphereCenter + direction * f_r;
    }

    float4 positionWorld = renderInfo.transformModelMatrix * float4(positionAnchor, 1.0);
    float4 positionView = renderInfo.viewMatrix * positionWorld;

    viewPosition[header + 0] =  positionView.x;
    viewPosition[header + 1] =  positionView.y;
    viewPosition[header + 2] =  positionView.z;
};
