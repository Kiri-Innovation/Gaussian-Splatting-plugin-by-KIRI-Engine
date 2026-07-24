#include "version.glsl"
#include "define.glsl"

layout(location = 0) in vec3 aPos;
out vec4 color; 
out vec2 texCoord;
out float depth;

#include "struct.glsl"
#include "uniforms.glsl"
#include "gaussian.glsl"
#include "colorGradient.glsl"
#include "bezierCurve.glsl"
#include "geometry.glsl"
#include "noise.glsl"

void main() {
  
    int depthIndex = int(texelFetch(u_depthIndexData , gl_InstanceID).r);

    GetSplatElement(depthIndex);

    vec3 positionLocal  =  u_splatElement.position;   
    //vec3 positionAnchor = (u_renderInfo.anchorModelMatrix * vec4(positionLocal, 1.0)).xyz;

    vec4 positionView = vec4(texelFetch(u_viewPositionBuffer, depthIndex * 3 + 0).r,
                             texelFetch(u_viewPositionBuffer, depthIndex * 3 + 1).r,
                             texelFetch(u_viewPositionBuffer, depthIndex * 3 + 2).r,
                             1.0);  
    vec4 positionAnchor4 = inverse(u_renderInfo.transformModelMatrix) * inverse(u_renderInfo.viewMatrix) * positionView;
    vec3 positionAnchor = positionAnchor4.xyz / positionAnchor4.w;
   
    //splat scale 
    if (u_renderInfo.splatScaleEnable == 1.0)
    {
        vec3 sizeNormalDir = (positionAnchor - u_renderInfo.splatScaleShapeCenter.xyz);
        sizeNormalDir /= u_renderInfo.splatScaleShapeScaleXYZ.xyz * PERCENT;
        float sizeNormalLength = length(sizeNormalDir);
        
        float sizeRadiusOuterBound = max(10 * u_renderInfo.splatScaleShapeSize, 0.0001);
        float sizeFeatherInnerBound = (1.0 - u_renderInfo.splatScaleShapeFeather) * sizeRadiusOuterBound;
        float sizeFeatherBand = max(sizeRadiusOuterBound - sizeFeatherInnerBound, 0.0001);
        
        float sizeLerpFactor = clamp((sizeNormalLength - sizeFeatherInnerBound) / sizeFeatherBand, 0.0, 1.0);
        float sizeRampFactor = 1.0 - GetValueFromBezierCurve(u_splatScaleRamp, sizeLerpFactor);
        float sizeFactor = mix( 0.0 , u_renderInfo.splatScaleSize, sizeRampFactor);
        u_splatElement.scale *= sizeFactor;
        u_splatElement.scale = max(vec3(0.0), u_splatElement.scale);

    }
    
    // splat dense
    // [0, 1]
    if (u_renderInfo.splatDenseEnable == 1.0)
    {
        vec3 denseNormalDir = (positionAnchor - u_renderInfo.splatDenseShapeCenter.xyz);
        denseNormalDir /= u_renderInfo.splatDenseShapeScaleXYZ.xyz * PERCENT;
        float denseNormalLength = length(denseNormalDir);
        float denseRadiusBound = max(10 * u_renderInfo.splatDenseShapeSize, 0.0001);
        
        float denseRadiusOuterBound = max(10 * u_renderInfo.splatDenseShapeSize, 0.0001);
        float denseFeatherInnerBound = (1.0 - u_renderInfo.splatDenseShapeFeather) * denseRadiusOuterBound;
        float denseFeatherBand = max(denseRadiusOuterBound - denseFeatherInnerBound, 0.0001);
        float denseLerpFactor =  clamp((denseNormalLength - denseFeatherInnerBound) / denseFeatherBand, 0.0, 1.0);
        float denseRampFactor = 1.0 - GetValueFromBezierCurve(u_splatDenseRamp, denseLerpFactor);
        
        float denseFactor = clamp(spatialHash(u_splatElement.position), 0.0, 1.0) ;
        float densethreshold = u_renderInfo.splatDenseDensity * PERCENT * denseRampFactor;
        //step(denseFactor, u_renderInfo.splatDenseDensity) *
        float denseMask = step(denseNormalLength, denseRadiusBound);
        u_splatElement.scale *= mix(1.0, step(denseFactor, densethreshold), denseMask);
    }

    
    // global splat scale crop
    if (u_renderInfo.advancedSplatCropMinScale < u_renderInfo.advancedSplatCropMaxScale)
    {    
            u_splatElement.scale *= 1.0 - step(u_renderInfo.advancedSplatCropMinScale * THOUSANDTH, u_splatElement.scale.x) *
                                      step(u_splatElement.scale.x, u_renderInfo.advancedSplatCropMaxScale * THOUSANDTH);
    }
    
    // base sh color
    color = vec4(ComputeColorFromSH(positionLocal), u_splatElement.opacity);
    
    // color gradient 
    if (u_renderInfo.colorEnable == 1.0)
    {        
        vec3 colorNormalDir = (positionAnchor - u_renderInfo.colorShapeCenter.xyz);
        colorNormalDir /= u_renderInfo.colorShapeScaleXYZ.xyz * PERCENT;
        float colorNormalLength = length(colorNormalDir);
        
        float colorRadiusOuterBound = max(10 * u_renderInfo.colorShapeSize, 0.0001);
        float colorFeatherInnerBound = (1.0 - u_renderInfo.colorShapeFeather) * colorRadiusOuterBound;
        float colorFeatherBand = max(colorRadiusOuterBound - colorFeatherInnerBound, 0.0001);
        
        float colorLerpFactor = clamp((colorNormalLength - colorFeatherInnerBound) / colorFeatherBand , 0.0, 1.0); 
        
        //float colorFeatherFactor = clamp(colorNormalLength / colorFeatherInnerRadius, 0.0, 1.0);
        
        //float colorGradientmask = step(colorNormalLength, colorRadiusBound);
        float colorRampFactor = 1.0 - GetValueFromBezierCurve(u_colorRamp, colorLerpFactor);
        
        vec3 gradientColor = mix(color.xyz, ComputeColorFromColorGradient(u_colorGradient, colorLerpFactor),  colorRampFactor ); // * colorGradientmask);
        color.xyz = gradientColor;
    }
    
    //opacity
    if (u_renderInfo.splatOpacityEnable == 1.0)
    {
        vec3 opacityNormalDir = (positionAnchor - u_renderInfo.splatOpacityShapeCenter.xyz);
        opacityNormalDir /= u_renderInfo.splatOpacityShapeScaleXYZ.xyz * PERCENT;
        float opacityNormalLength = length(opacityNormalDir);
        
        float opacityRadiusOuterBound = max(10 * u_renderInfo.splatOpacityShapeSize, 0.0001);
        float opacityFeatherInnerBound = (1.0 - u_renderInfo.splatOpacityShapeFeather) * opacityRadiusOuterBound;
        float opacityFeatherBand = max(opacityRadiusOuterBound - opacityFeatherInnerBound, 0.0001);
        
        float opacityLerpFactor = clamp((opacityNormalLength - opacityFeatherInnerBound) / opacityFeatherBand, 0.0, 1.0);
        float targetMaxOpacity = clamp(u_renderInfo.splatMaxOpacity * PERCENT, 0.0, 1.0);
        float targetMinOpacity = clamp(u_renderInfo.splatMinOpacity * PERCENT, 0.0, 1.0);
        float opacityRampFactor = 1.0 - GetValueFromBezierCurve(u_splatOpacityRamp, opacityLerpFactor);
        float opacityFactor = mix(targetMinOpacity ,targetMaxOpacity,  opacityRampFactor);
        color.a *= opacityFactor;
    }

    //crop
    if (u_renderInfo.cropEnable == 1.0)
    {
        vec3 cropNormalDir = (positionAnchor - u_renderInfo.cropShapeCenter.xyz);
        cropNormalDir /= u_renderInfo.cropShapeScaleXYZ.xyz * PERCENT;
        float cropNormalLength = length(cropNormalDir);
        float cropRadiusBound = max(10 * u_renderInfo.cropShapeSize, 0.0001);
        float cropMask = step(cropNormalLength, cropRadiusBound);
        cropMask = mix(1 - cropMask, cropMask, u_renderInfo.cropInvert);
        color = mix(color, vec4(0, 0, 0, 0), cropMask);
    }
    

    vec4 splatProject = ComputeSplatProject(positionView);

    gl_Position = splatProject;
    // [-near , -far] 
    depth = positionView.z;

}
