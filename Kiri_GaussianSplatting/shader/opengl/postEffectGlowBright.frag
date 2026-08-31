#include "version.glsl"
#include "uniforms.glsl"
#include "blur.glsl"

layout(location = 0) out vec4 outGlowBrightColor;

uniform sampler2D u_RawColorTexture;
in vec2 texCoord;

void main() {

    vec4 rawColor = texture(u_RawColorTexture, texCoord);
    float luminance = 0.299 * rawColor.r + 0.587 * rawColor.g + 0.114 * rawColor.b;
    float smoothness = u_renderInfo.advancedGlowSmooth;
    float brightUpperBound = clamp(1.0 - u_renderInfo.advancedGlowThreshold , 0.0 , 1.0);
    float mask = clamp((luminance - brightUpperBound) / smoothness, 0.0f, 1.0f);
    mask = mask * mask * (3 - 2 * mask);
    outGlowBrightColor = vec4(rawColor.rgb * rawColor.a * mask, rawColor.a * mask);
}