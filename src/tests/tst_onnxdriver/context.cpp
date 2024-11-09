#include <ctime>
#include <sstream>

#include <dsinfer/log.h>

#include <stdcorelib/global.h>
#include <stdcorelib/system.h>
#include <stdcorelib/console.h>

#include "context.h"

void log_report_callback(int level, const char *category, const char *fmt, va_list args) {
    namespace DS = dsinfer;
    namespace cho = std::chrono;

    auto current_dt = cho::system_clock::to_time_t(cho::system_clock::now());
    struct tm time_struct{};
#if defined(_WIN32) || defined(_WIN64)
    localtime_s(&time_struct, &current_dt);
#else
    localtime_r(&current_dt, &time_struct);
#endif
    auto dts = (std::stringstream() << std::put_time(&time_struct, "%Y-%m-%d %H:%M:%S")).str();

    int foreground, background;
    if (level <= DS::Log::Verbose) {
        foreground = stdc::Console::Default;
        background = stdc::Console::White;
    } else if (level <= DS::Log::Information) {
        foreground = stdc::Console::Blue | stdc::Console::Intensified;
        background = foreground;
    } else if (level <= DS::Log::Warning) {
        foreground = stdc::Console::Yellow;
        background = foreground;
    } else {
        foreground = stdc::Console::Red;
        background = foreground;
    }

    const char *sig = "I";
    switch (level) {
        case DS::Log::Trace:
            sig = "T";
            break;
        case DS::Log::Debug:
            sig = "D";
            break;
        case DS::Log::Verbose:
            sig = "V";
            break;
        case DS::Log::Warning:
            sig = "W";
            break;
        case DS::Log::Critical:
            sig = "C";
            break;
        case DS::Log::Fatal:
            sig = "F";
            break;
        default:
            break;
    }
    stdc::Console::printf(foreground, stdc::Console::Default, "[%s] %-15s", dts.c_str(), category);
    stdc::Console::printf(stdc::Console::Black, background, " %s ", sig);
    stdc::Console::printf(stdc::Console::Default, stdc::Console::Default, "  ");
    stdc::Console::vprintf(foreground, stdc::Console::Default, fmt, args);
}

Context::Context() : logger("onnxtest") {
    // Get basic directories
    appDir = stdc::System::applicationDirectory();
    defaultPluginDir =
        appDir.parent_path() / _TSTR("lib") / _TSTR("plugins") / _TSTR("dsinfer");

    // Set default plugin directories
    env.addPluginPath("com.diffsinger.InferenceDriver",
                      defaultPluginDir / _TSTR("inferencedrivers"));

}
