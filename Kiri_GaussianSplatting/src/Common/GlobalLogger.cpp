#include "Common/GlobalLogger.h"
#include "Common/Utils.h"
#include <filesystem> 
#include "iostream"
#ifdef  _WIN32
#include <Windows.h>
#endif 


GlobalLogger :: GlobalLogger() {
#if defined(__APPLE__)

    const char* appdata = std::getenv("HOME");
    std::filesystem::path log_dir = std::filesystem::path(appdata) / "Kiri_GaussianSplatting" / "logs";

#elif defined(_WIN32)
    const wchar_t* appdata = _wgetenv(L"LOCALAPPDATA");

    std::filesystem::path log_dir = std::filesystem::path(appdata) / L"Kiri_GaussianSplatting" / L"logs";
#endif
    std::filesystem::create_directories(log_dir);

    std::filesystem::path log_file = log_dir / "app.log";

    plog::init(
        plog::debug,
        //plog::error,
        log_file.string().c_str(),
        size_t(1024 * 1024 * 5),
        10);

    PLOGD << "Init GlobalLogger success";
    PLOGD << "Log will be saved in " << log_dir;

}

GlobalLogger :: ~GlobalLogger() {

}

