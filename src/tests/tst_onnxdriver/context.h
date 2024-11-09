#ifndef TST_ONNXDRIVER_UTILS_H
#define TST_ONNXDRIVER_UTILS_H

#include <cstdarg>
#include <filesystem>

#include <stdcorelib/console.h>

#include <dsinfer/environment.h>
#include <dsinfer/log.h>

void log_report_callback(int level, const char *category, const char *fmt, va_list args);

struct Context {
    std::filesystem::path appDir;
    std::filesystem::path defaultPluginDir;
    std::filesystem::path startupConfigPath;

    dsinfer::Environment env;
    dsinfer::Log::Category logger;

    Context();

    template <class... Args>
    static inline void info(const std::string &format, Args &&...args) {
        stdc::Console::printf(stdc::Console::Default, stdc::Console::Default, "%s\n",
                              stdc::formatN(format, args...).c_str());
    }

    template <class... Args>
    static inline void warning(const std::string &format, Args &&...args) {
        stdc::Console::printf(stdc::Console::Yellow, stdc::Console::Default, "%s\n",
                              stdc::formatN(format, args...).c_str());
    }

    template <class... Args>
    static inline void critical(const std::string &format, Args &&...args) {
        stdc::Console::printf(stdc::Console::Red, stdc::Console::Default, "%s\n",
                              stdc::formatN(format, args...).c_str());
    }
};

#endif // TST_ONNXDRIVER_UTILS_H