#include "define.metal"


// do not change the following member order for std 140 layout
struct RenderInfo {
    float4x4 modelMatrix; 
    float4x4 anchorModelMatrix;
    float4x4 transformModelMatrix;
    float4x4 viewMatrix;        
    float4x4 projectionMatrix;  

    float2 viewport;
    float focalPixelX;
    //float focalPixelY;
    
    int instanceCount;
    
    //int pad_0[3];

    float4 cameraPos;
    
    // do not change the following member order for std 140 layout
    float4 colorShapeCenter;
    float4 colorShapeScaleXYZ;
    int shDegree;
    float colorEnable;
    float colorShapeSize;
    float colorShapeFeather;

    float4 cropShapeCenter;
    float4 cropShapeScaleXYZ;
    float cropEnable;
    int cropInvert;
    float cropShapeSize;
    float cropShapeFeather;

    float4 splatScaleShapeCenter;
    float4 splatScaleShapeScaleXYZ;
    float splatScaleEnable;
    float splatScaleSize;
    float splatScaleShapeSize;
    float splatScaleShapeFeather;

    float4 splatNoiseShapeCenter;
    float4 splatNoiseShapeScaleXYZ;
    float splatNoiseEnable;
    float splatNoiseShapeSize;
    float splatNoiseShapeFeather;
    int splatNoiseOctaves;
    float splatNoisePersistence;
    float splatNoiseLacunarity;
    float splatNoiseStrength;
    float splatNoiseStrengthX;
    float splatNoiseStrengthY;
    float splatNoiseStrengthZ;
    float pad_0;


    float splatOpacityEnable;
    float4 splatOpacityShapeCenter;
    float4 splatOpacityShapeScaleXYZ;
    float splatOpacityShapeFeather;
    float splatOpacityShapeSize;
    float splatMaxOpacity;
    float splatMinOpacity;

    float4 splatDisplacementOffset;
    float4 splatDisplacementRotation;
    float4 splatDisplacementShapeScaleXYZ;
    float4 splatDisplacementShapeCenter;
    float splatDisplacementEnable;
    float splatDisplacementScale;
    float splatDisplacementShapeSize;
    float splatDisplacementShapeFeather;

    float4 splatDenseShapeCenter;
    float4 splatDenseShapeScaleXYZ;
    float splatDenseDensity;
    float splatDenseEnable;
    float splatDenseShapeSize;
    float splatDenseShapeFeather;

    float advancedSplatCropNear;
    float advancedSplatCropFar;
    float advancedCameraFocalLength;
    float advancedSplatCropMaxScale;

    float advancedSplatCropMinScale;
    float advancedDofEnable;
    float advancedDofFocusDistance;
    float advancedDofAperture;

    float advancedDofBlurLevel;
    float advancedGlowEnable;
    float advancedGlowBlendMode;
    float advancedGlowRadius;

    float advancedGlowThreshold;
    float advancedGlowSmooth;
    float splatInvertSphereEnable;
    float splatInvertSphereRaduis;

    float4 splatInvertSphereCenter;
    float splatInvertSphereIntensity;
    float splatInvertSphereDistance;
    float splatInvertSphereCompression;
    
    int splatCount;
    float focalPixelY;

};

struct BezierPoint
{
    float x;
    float y;
    float pad_0;
    float pad_1;
};

struct BezierCurveInfo
{
    BezierPoint bezierPointInfo[MAX_BEZIER_POINT_COUNT];
    int currentPointCount;
    int pad_0;
    int pad_1;
    int pad_2;
};
