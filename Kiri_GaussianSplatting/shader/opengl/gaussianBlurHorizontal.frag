#include "version.glsl"
#include "define.glsl"

in vec2 texCoord;

uniform sampler2D u_GlowBrightTexture;
uniform vec2 u_TexelSize;  // (1/width, 1/height)

out vec4 fragColor;

void main() {
    // 9-tap Gaussian weights for horizontal blur
    float weights[9] = float[](
        0.05, 0.09, 0.14, 0.19, 0.20, 0.19, 0.14, 0.09, 0.05
    );
    
    vec4 color = vec4(0.0);
    vec2 offset = vec2(u_TexelSize.x, 0.0);  // 只沿水平方向偏移
    
    for (int i = -4; i <= 4; i++)
    {
        vec2 sampleUV = texCoord + float(i) * offset;
        sampleUV = clamp(sampleUV, vec2(0.0), vec2(1.0));
        vec4 sampleColor = texture(u_GlowBrightTexture, sampleUV);
        color += sampleColor * weights[i + 4];
    }
    
    fragColor = color;

}