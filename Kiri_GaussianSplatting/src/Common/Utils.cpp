#include "Common/Utils.h"
#include <chrono>
#include <ctime>
#include <iomanip>
#include <mutex>
#if defined(_WIN32)
    #include <windows.h>
#endif
#if defined(__APPLE__)
    #include <CoreFoundation/CoreFoundation.h>
#endif
#include "Common/Global.h"

static std::mutex mtx;

void Log(const char* msg)
{
//#if defined(_DEBUG)

    auto now = std::chrono::system_clock::now();
    std::time_t tt = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};

#if defined(_WIN32)
    localtime_s(&tm, &tt);
#else
    localtime_r(&tt, &tm);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    std::lock_guard<std::mutex> lock(mtx);

    const char* safe_msg = (msg != nullptr) ? msg : "[null message]";

    g_log << "[" << oss.str() << "] " << safe_msg << std::endl;
    g_log.flush();

    do {} while (0);
}


void Log(const unsigned short* msg)
{
    /*
#if defined(_DEBUG)
    auto now = std::chrono::system_clock::now();
    std::time_t tt = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};

    std::wstring wstr = std::wstring(
        reinterpret_cast<const wchar_t*>(msg));
   // std::string str = WStringToUTF8(wstr);

#if defined(_WIN32)
    localtime_s(&tm, &tt);
#else
    localtime_r(&tt, &tm);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    std::lock_guard<std::mutex> lock(mtx);

    g_log << "[" << oss.str() << "] " << str << std::endl;
    g_log.flush();

#else
    do {} while (0);
#endif
*/
}

void Log(const std::string & msg)
{
#if defined(_DEBUG)
    auto now = std::chrono::system_clock::now();
    std::time_t tt = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};

#if defined(_WIN32)
    localtime_s(&tm, &tt);
#else
    localtime_r(&tt, &tm);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    std::lock_guard<std::mutex> lock(mtx);

    g_log << "[" << oss.str() << "] " << msg << std::endl;
    g_log.flush();

#else
    do {} while (0);
#endif
}
#if defined(_WIN32)
std::wstring utf8_to_utf16(const std::string& utf8)
{

    if (utf8.empty())
        return {};

    int len = MultiByteToWideChar(
        CP_UTF8,
        0,
        utf8.data(),
        (int)utf8.size(),
        nullptr,
        0);

    std::wstring utf16(len, 0);

    MultiByteToWideChar(
        CP_UTF8,
        0,
        utf8.data(),
        (int)utf8.size(),
        utf16.data(),
        len);

    return utf16;
}

std::string GBKToUTF8(const char* str)
{
    if (!str) return std::string();

    // GBK -> UTF-16
    int lenW = MultiByteToWideChar(
        CP_ACP,   // GBK/系统默认ANSI
        0,
        str,
        -1,
        NULL,
        0
    );

    if (lenW <= 0) return std::string();

    std::wstring wstr(lenW, L'\0');

    MultiByteToWideChar(
        CP_ACP,
        0,
        str,
        -1,
        &wstr[0],
        lenW
    );

    // UTF-16 -> UTF-8
    int lenA = WideCharToMultiByte(
        CP_UTF8,
        0,
        wstr.c_str(),
        -1,
        NULL,
        0,
        NULL,
        NULL
    );

    if (lenA <= 0) return std::string();

    std::string out(lenA, '\0');

    WideCharToMultiByte(
        CP_UTF8,
        0,
        wstr.c_str(),
        -1,
        &out[0],
        lenA,
        NULL,
        NULL
    );

    // 去掉末尾 '\0'
    if (!out.empty() && out.back() == '\0')
        out.pop_back();

    PLOGD << "UTF-8 " << out;
    return out;
}



#endif
