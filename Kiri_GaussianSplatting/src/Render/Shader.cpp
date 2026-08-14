#include "Render/Shader.h"
#include "generated/shader_autogen.h"


ShaderProgramSource GetShaderProgramSource(ShaderProgramType type) {
    ShaderProgramSource source;
    source.type = type;
    if (type == ShaderProgramType::PointAnimate) {
        
#if defined(_WIN32)
        source.comp = computeShaderCode;
#else 
        source.comp = metalCode;
#endif 
    }
    else if (type == ShaderProgramType::Main) {
        source.vert = mainVertCode;
        source.frag = mainFragCode;
    }
    else if (type == ShaderProgramType::PostEffectDof) {
        source.vert = postEffectDofVertCode;
        source.frag = postEffectDofFragCode;
    }
    else if (type == ShaderProgramType::PostEffectGlowBright) {
        source.vert = postEffectGlowBrightVertCode;
        source.frag = postEffectGlowBrightFragCode;
    }
    else if (type == ShaderProgramType::GaussianBlurVertical) {
        source.vert = gaussianBlurVerticalVertCode;
        source.frag = gaussianBlurVerticalFragCode;
    }
    else if (type == ShaderProgramType::GaussianBlurHorizontal) {
        source.vert = gaussianBlurHorizontalVertCode;
        source.frag = gaussianBlurHorizontalFragCode;
    }
    else if (type == ShaderProgramType::PostEffectGlowUpSample) {
        source.vert = postEffectGlowBrightUpsampleVertCode;
        source.frag = postEffectGlowBrightUpsampleFragCode;
    }
    else if (type == ShaderProgramType::PostEffectGlowBlend) {
        source.vert = postEffectGlowBlendVertCode;
        source.frag = postEffectGlowBlendFragCode;
    }
    return source;
}
