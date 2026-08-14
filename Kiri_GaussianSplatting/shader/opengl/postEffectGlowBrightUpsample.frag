#include "version.glsl"
#include "uniforms.glsl"

layout(location = 0) out vec4 outGlowBrightColor;

in vec2 texCoord;

uniform sampler2D u_GlowBrightMipmapTexture1;
uniform sampler2D u_GlowBrightMipmapTexture2;
uniform sampler2D u_GlowBrightMipmapTexture4;
uniform sampler2D u_GlowBrightMipmapTexture8;
uniform sampler2D u_GlowBrightMipmapTexture16;
uniform sampler2D u_GlowBrightMipmapTexture32;

void main() {
    vec4 mip0 = texture(u_GlowBrightMipmapTexture1, texCoord);
    vec4 mip1 = texture(u_GlowBrightMipmapTexture2, texCoord);
    vec4 mip2 = texture(u_GlowBrightMipmapTexture4, texCoord);
    vec4 mip3 = texture(u_GlowBrightMipmapTexture8, texCoord);
    vec4 mip4 = texture(u_GlowBrightMipmapTexture16, texCoord);
    vec4 mip5 = texture(u_GlowBrightMipmapTexture32, texCoord);
    
    float u_Scatter = min(u_renderInfo.advancedGlowRadius, 0.95);
    
    vec4 glow = mip5;                   
    glow = mix(mip4, glow, u_Scatter);  
    glow = mix(mip3, glow, u_Scatter);  
    glow = mix(mip2, glow, u_Scatter);  
    glow = mix(mip1, glow, u_Scatter);  
    glow = mix(mip0, glow, u_Scatter);  

    outGlowBrightColor = glow;
}