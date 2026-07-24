#pragma once

#include <glm/glm.hpp>
#include "string"
#include <memory>

#include "Render/ShaderInput.h"
#include "Render/GaussianModel.h"
#include "vector"
#include "mutex"


class GaussianRenderer {

public:
	GaussianRenderer();
	
	~GaussianRenderer();

	void Render(
				//PF_InData* in_data,
				//PF_ParamDef* params[],
				GaussianModel& gaussianModel,
				ShaderInput& shaderInput,
				const AEStreamValueInfo& streamValue);

	RenderResult GetRenderResult(const ShaderInput& shaderInput);

private :
	ShaderProgramInfo shaderProgramInfo;
	ShaderProgramInfo computeShaderProgramInfo;
	ShaderProgramInfo postEffectProgramInfo;

	RenderTargetInfo renderTargetInfo;
	SplatMeshInfo splatMeshInfo;
	RenderResult renderResult;
	std::recursive_mutex renderMutex;
	std::unordered_map<std::string , int> uniformLocCache;
	std::vector<float> posViewBuffer;
	std::span<float> posViewBufferSpan;
	unsigned int width  = 0;
	unsigned int height = 0;

	unsigned int renderFrameCount = 1;
	SSBOInfo colorGradientCursorSSBO;
	SSBOInfo positionSSBO;
	SSBOInfo viewPositionSSBO;
	UBOInfo  colorGradientCursorUBO;
	UBOInfo  renderInfoUBO;
	UBOInfo  bezierCurveUBO;

	int currentSplatCount = 0;

	void WriteUniformBuffer(const GaussianModel& gaussainModel,
							const ShaderInput& shaderInput);
	
	void SavePng(const char* filePath, const RenderResult& renderResult);
	void SaveJpg(const char* filePath, const RenderResult& renderResult);
	void GetColorGradientCursorUBO(
		const std::vector<ColorGradientInfoGpu>& colorGradientBlock,
		UBOInfo& colorGradientCursorUBO);
	void GetBezierCurveInfoUBO (
		const std::vector<BezierCurveInfoGpu>& bezierCurveBlock,
		UBOInfo& bezierCurveUBO);

	void GetRenderInfoUBO(const GaussianRenderInfo& renderInfo, UBOInfo& uboInfo);

	int  GetUniformLoc(const ShaderProgramInfo& shaderProgramInfo, const std::string& uniformName);
	int  GetUniformBinding(const std::string& uniformName, int resorceType);
	void ComputeAnimatedSplatData(
		const GaussianModel& gaussianModel, 
		const ShaderInput& shaderInput , 
		std::span<float>& posViewBufferSpan);
};

