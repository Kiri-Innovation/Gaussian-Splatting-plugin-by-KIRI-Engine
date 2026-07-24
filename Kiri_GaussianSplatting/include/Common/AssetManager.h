#pragma once

#include "vector"
#include "string"
#include <memory>
#include "AEConfig.h"
#include "Render/OpenGLManager.h"
#include "array"
#include "Render/GaussianModel.h"
#include "Render/GaussianTypes.h"
#include "mutex"
#include "chrono"
#include "shared_mutex"

typedef struct AssetCacheInfo {
	std::shared_ptr<GaussianModel> gaussianModel;
	std::chrono::high_resolution_clock::time_point lastUseTime;
}AssetCacheInfo;


class AssetManager {
	public :
		AssetManager();
		~AssetManager();
		std::shared_ptr<GaussianModel> GetGaussianModel(const std::string& filePath);
		void CleanUp();

private:
	std::unordered_map<std::string, AssetCacheInfo> assetCache;
	//std::mutex mutex;
	std::shared_mutex rwMutex;
};