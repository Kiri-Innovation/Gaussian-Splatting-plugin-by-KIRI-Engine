#pragma once

#include "string"

enum ShaderProgramType {
	PointAnimate,
	Main,
	PostEffectDof,

    // gaussian blur
    GaussianBlurVertical,
    GaussianBlurHorizontal,

    //glow
    PostEffectGlowBright,
    PostEffectGlowUpSample,
    PostEffectGlowBlend
};

struct ShaderProgramSource {
    ShaderProgramType type;

    const char* vert = nullptr;
    const char* frag = nullptr;
    const char* comp = nullptr;
};

ShaderProgramSource GetShaderProgramSource(ShaderProgramType type);



