#pragma once

#include <string>
#include <sstream>
#include <fstream> 

#include <plog/Log.h>
#include <plog/Initializers/RollingFileInitializer.h>




void Log(const char* msg);

void Log(const unsigned short* msg);

void Log(const std::string& msg);

std::wstring utf8_to_utf16(const std::string& utf8);

std::string GBKToUTF8(const char* str);

