#include "Log.hpp"
#include <iostream>
#include <cstdarg>
#include <sstream>
#include <windows.h>

void Log::Debug(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    LogMessage(Level::Debug, fmt, args);
    va_end(args);
}

void Log::Info(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    LogMessage(Level::Info, fmt, args);
    va_end(args);
}

void Log::Warning(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    LogMessage(Level::Warning, fmt, args);
    va_end(args);
}

void Log::Error(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    LogMessage(Level::Error, fmt, args);
    va_end(args);
}

void Log::Critical(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    LogMessage(Level::Critical, fmt, args);
    va_end(args);
}

void Log::LogMessage(Level level, const char *fmt, va_list args) {
    std::string message = FormatString(fmt, args);
    std::string levelStr = GetLevelString(level);

#ifdef _DEBUG
    std::ostream &outStream = (level == Level::Error || level == Level::Critical) ? std::cerr : std::cout;
    outStream << "[" << levelStr << "] " << message << std::endl;
#else
    if (level == Level::Error || level == Level::Critical) {
        std::ostringstream oss;
        oss << "[" << levelStr << "] " << message << std::endl;
        MessageBox(NULL, oss.str().c_str(), "WebFrame", MB_OK);
    }
#endif
}

std::string Log::FormatString(const char *fmt, va_list args) {
    va_list args_copy;
    va_copy(args_copy, args);

    int size = std::vsnprintf(nullptr, 0, fmt, args_copy);
    va_end(args_copy);

    if (size <= 0) return "Formatting Error: " + std::string(fmt);

    std::string buffer(size, '\0');
    std::vsnprintf(&buffer[0], buffer.size() + 1, fmt, args);
    return buffer;
}

std::string Log::GetLevelString(Level level) {
    switch (level) {
    case Level::Debug:
        return "DEBUG";
    case Level::Info:
        return "INFO";
    case Level::Warning:
        return "WARNING";
    case Level::Error:
        return "ERROR";
    case Level::Critical:
        return "CRITICAL";
    default:
        return "UNKNOWN";
    }
}
