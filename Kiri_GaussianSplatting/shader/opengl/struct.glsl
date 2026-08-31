#include "define.glsl"

// do not change the following member order for std 140 layout
struct RenderInfo {
    mat4 modelMatrix; 
    mat4 anchorModelMatrix;
    mat4 transformModelMatrix;
    mat4 viewMatrix;        
    mat4 projectionMatrix;  

    vec2 viewport;
    float focalPixelX;
    //float focalPixelY;
    
    int instanceCount;
    
    //int pad_0[3];

    vec4 cameraPos;
    
    // do not change the following member order for std 140 layout
    vec4 colorShapeCenter;
    vec4 colorShapeScaleXYZ;
    int shDegree;
    float colorEnable;
    float colorShapeSize;
    float colorShapeFeather;

    vec4 cropShapeCenter;
    vec4 cropShapeScaleXYZ;
    float cropEnable;
    int cropInvert;
    float cropShapeSize;
    float cropShapeFeather;

    vec4 splatScaleShapeCenter;
    vec4 splatScaleShapeScaleXYZ;
    float splatScaleEnable;
    float splatScaleSize;
    float splatScaleShapeSize;
    float splatScaleShapeFeather;

    vec4  splatNoiseShapeCenter;
    vec4  splatNoiseShapeScaleXYZ;
    float splatNoiseEnable;
    float splatNoiseShapeSize;
    float splatNoiseShapeFeather;
    int   splatNoiseOctaves;
    float splatNoisePersistence;
    float splatNoiseLacunarity;
    float splatNoiseStrength;
    float splatNoiseStrengthX;
    float splatNoiseStrengthY;
    float splatNoiseStrengthZ;
    float pad_0;

    float splatOpacityEnable;
    vec4 splatOpacityShapeCenter;
    vec4 splatOpacityShapeScaleXYZ;
    float splatOpacityShapeFeather;
    float splatOpacityShapeSize;
    float splatMaxOpacity;
    float splatMinOpacity;

    vec4 splatDisplacementOffset;
    vec4 splatDisplacementRotation;
    vec4 splatDisplacementShapeScaleXYZ;
    vec4 splatDisplacementShapeCenter;
    float splatDisplacementEnable;
    float splatDisplacementScale;
    float splatDisplacementShapeSize;
    float splatDisplacementShapeFeather;
    
    vec4 splatDenseShapeCenter;
    vec4 splatDenseShapeScaleXYZ;
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

    vec4 splatInvertSphereCenter;
    float splatInvertSphereIntensity;
    float splatInvertSphereDistance;
    float splatInvertSphereCompression;
    
    float advancedGlowEnable;
    float advancedGlowBlendMode;
    float advancedGlowRadius;
    float advancedGlowThreshold;
    float advancedGlowSmooth;
    
    int splatCount;
    float focalPixelY;
};

struct SplatElement {
	vec3 position;
	vec3 sh[16];
    float opacity;
	vec3 scale;
	vec4 quat;
};

struct SplatElementF {
	float position[3];
	float sh[16*3];
    float opacity;
	float scale[3];
	float quat[4];
};

float SH_C0 = 0.28209479177387814;
float SH_C1 = 0.4886025119029199f;

const float SH_C2[5] = float[](
    1.0925484305920792,
    -1.0925484305920792,
    0.31539156525252005,
    -1.0925484305920792,
    0.5462742152960396
);

const float SH_C3[7] = float[](
    -0.5900435899266435,
    2.890611442640554,
    -0.4570457994644658,
    0.3731763325901154,
    -0.4570457994644658,
    1.445305721320277,
    -0.5900435899266435
);

struct ColorGradientCursor
{
    float lerpFactor;
    float R;
    float G;
    float B;
};

struct ColorGradientInfo
{
    ColorGradientCursor colorGradientCursor[MAX_COLOR_GRADIENT_COUNT];
    int currentCursorCount;
    int pad_0;
    int pad_1;
    int pad_2;
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
