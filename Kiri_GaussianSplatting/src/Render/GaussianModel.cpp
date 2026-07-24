#include "Render/GaussianModel.h"
#include "Common/Utils.h"
#include "Common/Global.h"
#include <bit>
#include <chrono>
#include <filesystem>
#include <algorithm>
#include <execution>
#include <cmath>
#include <cstdlib>
#include <oneapi/tbb/blocked_range.h>
#include <oneapi/tbb/parallel_for.h>
#include <oneapi/tbb/parallel_sort.h>

GaussianModel::GaussianModel() {
	PLOGI <<"GaussianModel Constructor";
	if (countBuffer.size() != bucketCount) {
		countBuffer.resize(bucketCount);
	}
}


GaussianModel::~GaussianModel() {
	PLOGI << "~GaussianModel";
	g_openGLManager.DeleteTextureBuffer(tboInfo);
	g_openGLManager.DeleteTextureBuffer(depthIndexTboInfo);
	g_openGLManager.DeleteSSBO(SSBOinfo);
}


// called in  3DGS_PF.cpp Render fuction
void GaussianModel::Init(const std::vector<StandardGaussian>& gaussianData)
{
	/*
	PLOGI <<"GaussianModel::Init " << path;

#ifdef _WIN32
	std::wstring  wpath = utf8_to_utf16(path);
	if (std::filesystem::exists(wpath) == false) {
#else
	if (std::filesystem::exists(path) == false) {
#endif

		PLOGI <<"input path is not exists";
		g_openGLManager.DeleteTextureBuffer(tboInfo);
		g_openGLManager.DeleteTextureBuffer(depthIndexTboInfo);
		g_openGLManager.DeleteSSBO(SSBOinfo);
		splatCount = 0;
		return;
	}

	if (currentPlyPath == path){
		PLOGI <<"input paht is tame with current path" << path;
		return;
	}

	currentPlyPath = path;
	

	std::vector<StandardGaussian> gaussianData = LoadPly(path);
	*/

	splatCount = gaussianData.size();
	if (splatCount != 0) {
		//if (zViewBuffer.size() != splatCount) {
		//	zViewBuffer.resize(splatCount);
		//}
		//if (depthBuffer.size() != splatCount) {
		//	depthBuffer.resize(splatCount);
		//}
		if (depthIndexBuffer.size() != splatCount) {
			depthIndexBuffer.resize(splatCount);

		}
		if (position.size() != splatCount * 3) {
			position.resize(splatCount * 3);
		}
		//if (sortKeyBuffer.size() != splatCount) {
		//	sortKeyBuffer.resize(splatCount);
		//}
		PLOGI <<"Init GaussianModel data to GPU" ;
		int memSize = splatCount * sizeof(StandardGaussian);

		//g_openGLManager.DeleteTextureBuffer(tboInfo);
		//tboInfo = g_openGLManager.CreateTextureBuffer(memSize, (float*)gaussianData.data());

		//PLOGI <<"CreateSSBO for GaussianModel");
		//g_openGLManager.DeleteSSBO(SSBOinfo);
		//SSBOinfo = g_openGLManager.CreateSSBO(sizeof(StandardGaussian) * splatCount , gaussianData.data());
		//PLOGI <<"CreateSSBO for GaussianModel success");

		PLOGI <<"CreateTextureBuffer for GaussianModel" << splatCount;
		g_openGLManager.DeleteTextureBuffer(tboInfo);
		tboInfo = g_openGLManager.CreateTextureBuffer(sizeof(StandardGaussian) * splatCount, gaussianData.data(), true);
		PLOGI <<"CreateTextureBuffer for GaussianModel success";

		PLOGI <<"CreateTextureBuffer for depthIndexBuffer";
		g_openGLManager.DeleteTextureBuffer(depthIndexTboInfo);
		depthIndexTboInfo = g_openGLManager.CreateTextureBuffer(splatCount * sizeof(int), depthIndexBuffer.data() , false);
		PLOGI <<"CreateTextureBuffer for depthIndexBuffer success";

		PLOGI <<"CreateTextureBuffer for viewPositionBuffer";
		g_openGLManager.DeleteTextureBuffer(viewPositionBufferInfo);
		viewPositionBufferInfo = g_openGLManager.CreateTextureBuffer(splatCount * sizeof(float) * 3, position.data(), false);
		g_openGLManager.GetGLError();
		PLOGI <<"CreateTextureBuffer for viewPositionBuffer success";

		//vec4
		//std::vector<float> tempBuffer;
		//tempBuffer.resize(splatCount * 4);
		//std::fill(tempBuffer.begin(), tempBuffer.end(), 1.0f);
		//g_openGLManager.DeleteTextureBuffer(testBuffer);
		//testBuffer = g_openGLManager.CreateTextureBuffer(tempBuffer.size() * sizeof(int), tempBuffer.data());

		for (int i = 0; i < gaussianData.size() ; i++) {
			 memcpy(&position[i * 3], &gaussianData[i].pos[0], 3 * sizeof(float));
		}

		PLOGI <<"Init GaussianModel success {}" << splatCount;

	}
	else {

		PLOGI <<"Init GaussianModel fail {}" << splatCount;
	}
}

