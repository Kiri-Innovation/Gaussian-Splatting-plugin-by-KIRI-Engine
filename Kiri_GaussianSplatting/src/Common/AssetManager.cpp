#include<Common/AssetManager.h>
#include<mutex>
#include "Common/Utils.h"
#include "IO/PlyImporter.h"  

AssetManager::AssetManager(){

}
AssetManager::~AssetManager() {

}

/// <summary>
/// AssetManager use LRU to manage memory
/// </summary>
/// <param name="filePath"></param>
/// <returns></returns>
std::shared_ptr<GaussianModel> AssetManager::GetGaussianModel(const std::string& filePath) {
    PLOGI <<"AssetManager::GetGaussianModel";
    auto currentTime = std::chrono::high_resolution_clock::now();
    {
       // std::lock_guard<std::recursive_mutex> lock(assetMutex);
        //std::lock_guard<std::mutex> lock(mutex);
        std::shared_lock<std::shared_mutex> lock(rwMutex);
        PLOGI <<"AssetManager::GetGaussianModel check cache " << filePath;
        auto it = assetCache.find(filePath);
        if (it != assetCache.end()) {
            it->second.lastUseTime = currentTime;
            return it->second.gaussianModel;
        }
    }

    PLOGI <<"AssetManager::GetGaussianModel begine new GaussianModel " << filePath;
    auto newModel = std::make_shared<GaussianModel>();
    std::vector<StandardGaussian> gaussianData;
    if (PlyImporter::Import(filePath, gaussianData) == false) {
        PLOGE << "Can not import " << filePath;
        return nullptr;
    }
    newModel->Init(gaussianData);

    {
        //std::lock_guard<std::mutex> lock(mutex);
        std::lock_guard<std::shared_mutex> lock(rwMutex);

        auto it = assetCache.find(filePath);
        if (it != assetCache.end()) {
            it->second.lastUseTime = currentTime;
            return it->second.gaussianModel; 
        }
        AssetCacheInfo info;
        info.gaussianModel = newModel;
        info.lastUseTime = currentTime;
        assetCache[filePath] = info;
    }

    return newModel;
}



 void AssetManager:: CleanUp() {

     auto currentTime = std::chrono::high_resolution_clock::now();
     for (auto it = assetCache.begin(); it != assetCache.end(); ) {

         auto duration = currentTime - it->second.lastUseTime;
         double seconds = std::chrono::duration<double>(duration).count();

         if (seconds > 10.0) {
             it = assetCache.erase(it);
         }
         else {
             ++it; 
         }
     }
}
