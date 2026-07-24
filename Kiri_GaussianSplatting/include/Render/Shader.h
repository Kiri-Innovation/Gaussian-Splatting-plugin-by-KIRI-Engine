#pragma once

#include "string"

enum ShaderProgramType {
	PointAnimate,
	Main,
	PostEffect,
};

struct ShaderProgramSource {
    ShaderProgramType type;

    const char* vert = nullptr;
    const char* frag = nullptr;
    const char* comp = nullptr;
};

ShaderProgramSource GetShaderProgramSource(ShaderProgramType type);



