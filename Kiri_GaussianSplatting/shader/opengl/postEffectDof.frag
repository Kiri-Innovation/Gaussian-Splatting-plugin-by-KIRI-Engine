#include "version.glsl"
#include "uniforms.glsl"
#include "blur.glsl"

layout(location = 0) out vec4 outFinalColor;

uniform sampler2D u_RawColorTexture;
uniform sampler2D u_DepthTexture;

in vec2 texCoord;

void main() {

    vec4 rawColor = texture(u_RawColorTexture, texCoord);
    // real accum depth
    // [near , far ] -> [ far , 0 ]
    //float z = u_renderInfo.advancedSplatCropFar - max(texture(u_DepthTexture, texCoord).r, 0.0001);
    float z =  max(texture(u_DepthTexture, texCoord).r, 0.0001);
    
    //// auto focus
    //float Z_focus     = u_renderInfo.advancedSplatCropFar - u_renderInfo.advancedDofFocusDistance;
    float Z_focus       = u_renderInfo.advancedDofFocusDistance;
    float blurLevel     = max(u_renderInfo.advancedDofBlurLevel, 0.1);
    float aperture      = max(u_renderInfo.advancedDofAperture, 0.1);
    float maxBlurRadius = 20.f * blurLevel * PERCENT;
    
    float coc = abs(z - Z_focus) / z ;
    
    float pixelRadius = clamp(coc *  aperture, 0.0, maxBlurRadius);
    float pixelRadius01 = pixelRadius / maxBlurRadius;
    //// pixel coord -> uv
    vec2 texelSize = 1.0 / vec2(textureSize(u_RawColorTexture, 0));
    
    vec4 blurColor = vec4(0.0);
    float totalWeight = 0.0;
    float sigma = max(pixelRadius * 0.5, 1.0) ;
    
    for (float y = -pixelRadius; y <= pixelRadius; y++)
    {
        for (float x = -pixelRadius; x <= pixelRadius; x++)
        {
            vec2 offset = vec2(float(x), float(y));
            float dist = length(offset);
            vec2 sampleUV = texCoord + offset * texelSize;
            vec4 sampleColor = texture(u_RawColorTexture, sampleUV);
            float sampleZ = texture(u_DepthTexture, sampleUV).r;
            
            float sampleCoc = abs(sampleZ - Z_focus) / sampleZ;
            float samplePixelRadius01 = clamp(sampleCoc * aperture, 0.0, maxBlurRadius) / maxBlurRadius;
            
            float gaussianWeight = GaussianWeight(dist, sigma);
            
            float cocWeight = clamp(
                1.0 - abs(pixelRadius01 - samplePixelRadius01),
                0.0,
                1.0
            );
            
            float weight = gaussianWeight * cocWeight;
            
            blurColor += sampleColor * weight;
            totalWeight += weight;
        }
    }
    blurColor /= totalWeight;
    float renderZ = (z - u_renderInfo.advancedSplatCropNear) / (u_renderInfo.advancedSplatCropFar - u_renderInfo.advancedSplatCropNear);
    //outFinalColor = vec4(vec3(renderZ), rawColor.a);
    //outFinalColor = vec4(vec3(pixelRadius01), rawColor.a);
    outFinalColor = blurColor;

}