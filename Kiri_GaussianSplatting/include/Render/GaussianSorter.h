#pragma once 
#include "Render/GaussianTypes.h"
#include "Render/GaussianModel.h"
#include "Render/ShaderInput.h"


enum SortComputeType {
	CPU,
	GPU
};

class GaussianSorter {
public:
	GaussianSorter();
	~GaussianSorter();

	static void Sort(
		GaussianModel& gaussianModel,
		ShaderInput& shaderInput,
		const AEStreamValueInfo& streamValue,
		std::span<float> posViewBufferSpan,
		SortComputeType sortComputeType);
private :
	static void SortOnCpu(
		GaussianModel& gaussianModel,
		ShaderInput& shaderInput,
		const AEStreamValueInfo& streamValue,
		std::span<float> posViewBufferSpan);


	static void SortOnGpu(
		GaussianModel& gaussianModel,
		ShaderInput& shaderInput,
		const AEStreamValueInfo& streamValue,
		std::span<float> posViewBufferSpan);
};