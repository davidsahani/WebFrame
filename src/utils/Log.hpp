#pragma once
#include <string>

class Log {
  public:
    enum class Level {
        Debug,
        Info,
        Warning,
        Error,
        Critical
    };

    static void Debug(const char *fmt, ...);
    static void Info(const char *fmt, ...);
    static void Warning(const char *fmt, ...);
    static void Error(const char *fmt, ...);
    static void Critical(const char *fmt, ...);

  public:
    static void SetWindowName(const std::string &name) { windowName = name; };
    static void SetLogFile(const std::string &filename) { logFilename = filename; };

  private:
    static void LogMessage(Level level, const char *fmt, va_list args);
    static std::string FormatString(const char *fmt, va_list args);
    static std::string GetLevelString(Level level);
    static std::string GetTimestamp();

  private:
    static std::string logFilename;
    static std::string windowName;
    static constexpr char default_WindowName[] = "WebFrame";
};
