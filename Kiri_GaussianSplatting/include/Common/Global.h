#pragma once

#include "Common/Utils.h"
//#include "Render/GaussianModel.h"
#include "Render/OpenGLManager.h"
#include "Render/GaussianRenderer.h"
#include "AssetManager.h"
#include "Render/MetalManager.h"
#include "Common/GlobalLogger.h"
#include <memory>

//extern MetalManager     g_metalManager;


extern AEGP_PluginID    g_pluginID;
extern SPBasicSuite*    g_pica_basicP;
#if defined(__APPLE__)
extern MetalManager     g_metalManager;
#endif
extern GaussianRenderer g_gaussianRenderer;
extern AssetManager     g_assetManager;
extern OpenGLManager    g_openGLManager;
extern GlobalLogger     g_logger;
extern std::ofstream    g_log;
