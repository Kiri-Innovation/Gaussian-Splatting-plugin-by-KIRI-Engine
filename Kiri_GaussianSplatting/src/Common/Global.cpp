#include "Common/Global.h"

std::ofstream g_log(
    
    "/Users/kiri/Desktop/runtime.log",
    std::ios::out | std::ios::app
);


AEGP_PluginID g_pluginID = 0;

SPBasicSuite* g_pica_basicP = NULL;

GlobalLogger g_logger = GlobalLogger();

OpenGLManager g_openGLManager = OpenGLManager();

GaussianRenderer g_gaussianRenderer = GaussianRenderer();

AssetManager g_assetManager = AssetManager();

#if defined(__APPLE__)
MetalManager g_metalManager = MetalManager();
#endif
