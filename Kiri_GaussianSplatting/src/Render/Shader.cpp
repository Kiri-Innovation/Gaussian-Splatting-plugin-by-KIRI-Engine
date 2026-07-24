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
    else if (type == ShaderProgramType::PostEffect) {
        source.vert = postEffectVertCode;
        source.frag = postEffectFragCode;
    }

    return source;
}
