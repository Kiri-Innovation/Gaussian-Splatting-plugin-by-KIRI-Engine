#include "version.glsl"
#include "uniforms.glsl"
layout(location = 0) out vec4 outGlowBrightColor;

in vec2 texCoord;

uniform sampler2D u_RawColorTexture;
uniform sampler2D u_GlowBrightTexture;

void main() {
    
    vec4 rawColor = texture(u_RawColorTexture, texCoord);
    vec4 glowColor = texture(u_GlowBrightTexture, texCoord);
    
    if (u_renderInfo.advancedGlowBlendMode == 0)
    {
        outGlowBrightColor = glowColor + rawColor;
    }
    else
    {
        outGlowBrightColor = 1.0 - (1.0 - glowColor) * (1.0 - rawColor);
    }
}
