#include "version.glsl"
#include "uniforms.glsl"

layout(location = 0) out vec4 outFinalColor;
layout(location = 1) out vec4 outColor;
layout(location = 2) out vec4 outDepth;

in vec4 color ;
// [-near , -far]
in float depth;
in vec2 texCoord;

void main() {
    float A = dot(texCoord, texCoord);
    if (A > 4.0) 
        discard;
    // [0.0 , 1.0]
    float near = u_renderInfo.advancedSplatCropNear;
    float far  = u_renderInfo.advancedSplatCropFar;  
    // [0 , 1]
    // only use to check depth map 
    float t = (-depth - near) / (far - near);
    outColor = vec4(color.rgb, color.a * exp(-A));
    if (u_renderInfo.advancedDofEnable == 1.0)
    {
        outDepth = vec4(vec3(-depth), color.a * exp(-A)); //vec4(vec3(t), color.a * exp(-A));
    }

}