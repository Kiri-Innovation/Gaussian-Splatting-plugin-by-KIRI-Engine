

#include "struct.glsl"

uniform isamplerBuffer u_depthIndexData;
uniform samplerBuffer u_TBOsplats;
uniform samplerBuffer u_viewPositionBuffer;

layout( std140 ) uniform RenderInfoBuffer {
    RenderInfo u_renderInfo;
};

layout( std140 ) uniform ColorGradientBuffer {
    ColorGradientInfo u_colorGradient;
};

layout( std140 ) uniform BezierCurveBuffer {
    BezierCurveInfo u_colorRamp;
    BezierCurveInfo u_splatScaleRamp;
    BezierCurveInfo u_splatOpacityRamp;
    BezierCurveInfo u_splatDisplacementOffsetRamp;
    BezierCurveInfo u_splatDisplacementScaleRamp;
    BezierCurveInfo u_splatDisplacementRotationRamp;
    BezierCurveInfo u_splatDenseRamp;
};


SplatElement u_splatElement;
