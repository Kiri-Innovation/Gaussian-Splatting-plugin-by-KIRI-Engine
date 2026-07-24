#pragma once

#include "Render/GaussianTypes.h"
#include "UI/UIDataType.h"

struct ShaderInput {
	int splatEnable;
	GaussianRenderInfo renderBlock;
	std::vector<ColorGradientInfoGpu> colorGradientBlock;
	std::vector<BezierCurveInfoGpu>   bezierCurveBlock;
};