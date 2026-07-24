#include "Render/GaussianSorter.h"

#include <oneapi/tbb/blocked_range.h>
#include <oneapi/tbb/parallel_for.h>
#include <oneapi/tbb/parallel_sort.h>

#include "Render/ShaderInput.h"
#include "Common/Global.h"
#include "Common/Utils.h"

GaussianSorter::GaussianSorter()
{
}

GaussianSorter::~GaussianSorter()
{
}

void GaussianSorter::Sort(
	GaussianModel& gaussianModel ,
	ShaderInput& shaderInput, 
	const AEStreamValueInfo& streamValue,
	std::span<float> posViewBufferSpan,
	SortComputeType sortComputeType)
{
	if (sortComputeType == SortComputeType::CPU) {
		SortOnCpu(gaussianModel , shaderInput , streamValue , posViewBufferSpan);
	}
	else if(sortComputeType == SortComputeType::GPU) {
		// TODO: 

	}

}

void GaussianSorter::SortOnCpu(
	GaussianModel& gaussianModel,
	ShaderInput& shaderInput,
	const AEStreamValueInfo& streamValue,
	std::span<float> posViewBufferSpan)
{
	PLOGD << "SortOnCpu";

	auto start = std::chrono::high_resolution_clock::now();
	int viewableCount = 0;
	int splatCount = shaderInput.renderBlock.splatCount;
	std::vector<int>& depthIndexBuffer =        gaussianModel.depthIndexBuffer;


	auto firstLoopStart = std::chrono::high_resolution_clock::now();
	for (int i = 0; i < splatCount; i++) {
		float zView = posViewBufferSpan[i * 3 + 2];
		if (zView < 0 && zView > -streamValue.advancedSplatCropFar && zView < -streamValue.advancedSplatCropNear) {
			depthIndexBuffer[viewableCount] = i;
			viewableCount++;
		}
	}
	auto firstLoopEnd = std::chrono::high_resolution_clock::now();


	auto secondLoopStart = std::chrono::high_resolution_clock::now();
	oneapi::tbb::parallel_sort(depthIndexBuffer.begin(), depthIndexBuffer.begin() + viewableCount,
		[posViewBufferSpan](int a, int b) {
			return posViewBufferSpan[a * 3 + 2] < posViewBufferSpan[b * 3 + 2];
		});
	auto secondLoopEnd = std::chrono::high_resolution_clock::now();

	auto thirdLoopStart = std::chrono::high_resolution_clock::now();
	glBindBuffer(GL_TEXTURE_BUFFER, gaussianModel.depthIndexTboInfo.TBO);
	glBufferSubData(GL_TEXTURE_BUFFER, 0, viewableCount * sizeof(int), depthIndexBuffer.data());
	//{
	//	const int debugCount = std::min(splatCount * 3, 10);
	//	std::vector<int> debugdepthIndex(debugCount);
	//	glGetBufferSubData(GL_TEXTURE_BUFFER, 0, debugCount * sizeof(int), debugdepthIndex.data());
	//
	//	std::ostringstream debugOss;
	//	debugOss << "debugdepthIndex first " << debugCount << " values:";
	//	for (int i = 0; i < debugCount; ++i) {
	//		debugOss << " " << debugdepthIndex[i];
	//	}
	//	PLOGI << debugOss.str();
	//}

	glBindBuffer(GL_TEXTURE_BUFFER, gaussianModel.viewPositionBufferInfo.TBO);
	glBufferSubData(GL_TEXTURE_BUFFER, 0, posViewBufferSpan.size_bytes(), posViewBufferSpan.data());
	//{
	//	const int debugCount = std::min(splatCount * 3, 10);
	//	std::vector<float> debugViewPosition(debugCount);
	//	glGetBufferSubData(GL_TEXTURE_BUFFER, 0, debugCount * sizeof(float), debugViewPosition.data());
	//
	//	std::ostringstream debugOss;
	//	debugOss << "u_viewPositionBuffer first " << debugCount << " values:";
	//	for (int i = 0; i < debugCount; ++i) {
	//		debugOss << " " << debugViewPosition[i];
	//	}
	//	PLOGI << debugOss.str();
	//}

	shaderInput.renderBlock.instanceCount = viewableCount;

	auto thirdLoopEnd = std::chrono::high_resolution_clock::now();


	auto end = std::chrono::high_resolution_clock::now();
	double totalMs = std::chrono::duration<double, std::milli>(end - start).count();
	double firstLoopMs = std::chrono::duration<double, std::milli>(firstLoopEnd - firstLoopStart).count();
	double secondLoopMs = std::chrono::duration<double, std::milli>(secondLoopEnd - secondLoopStart).count();
	double thirdLoopMs = std::chrono::duration<double, std::milli>(thirdLoopEnd - thirdLoopStart).count();

	std::ostringstream oss;
	auto appendStageTiming = [&](const char* stageName, double stageMs) {
		double relativePercent = totalMs > 0.0 ? (stageMs / totalMs) * 100.0 : 0.0;
		double fps = stageMs > 0.0 ? 1000.0 / stageMs : 0.0;
		oss << stageName
			<< " : " << stageMs << " ms"
			<< " | " << relativePercent << "%"
			<< " | " << fps << " fps"
			<< std::endl;
		};

	oss.str("");
	oss << "ComputeDepthBuffer Stage Timing" << std::endl;
	appendStageTiming("FirstLoop", firstLoopMs);
	appendStageTiming("SecondLoop", secondLoopMs);
	appendStageTiming("ThirdLoop", thirdLoopMs);
	oss << "Total"
		<< " : " << totalMs << " ms"
		<< " | 100%"
		<< " | " << (totalMs > 0.0 ? 1000.0 / totalMs : 0.0) << " fps"
		<< std::endl;
	PLOGI << oss.str();

	PLOGD << "Finish SortOnCpu " << viewableCount;
	
	g_openGLManager.GetGLError();
}
