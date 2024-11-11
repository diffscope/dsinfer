#include <ctime>
#include <sstream>

#include <dsinfer/log.h>

#include <stdcorelib/global.h>
#include <stdcorelib/system.h>
#include <stdcorelib/console.h>

#include "context.h"

void log_report_callback(int level, const char *category, const char *fmt, va_list args) {
    using namespace dsinfer;
    using namespace stdc;

    auto t = std::time(nullptr);
    auto tm = std::localtime(&t);

    std::stringstream ss;
    ss << std::put_time(tm, "%Y-%m-%d %H:%M:%S");
    auto dts = ss.str();

    int foreground, background;
    if (level <= Log::Verbose) {
        foreground = console::plain;
        background = console::white;
    } else if (level <= Log::Information) {
        foreground = stdc::console::blue | stdc::console::intensified;
        background = foreground;
    } else if (level <= Log::Warning) {
        foreground = console::yellow;
        background = foreground;
    } else {
        foreground = console::red;
        background = foreground;
    }

    const char *sig = "I";
    switch (level) {
        case Log::Trace:
            sig = "T";
            break;
        case Log::Debug:
            sig = "D";
            break;
        case Log::Verbose:
            sig = "V";
            break;
        case Log::Warning:
            sig = "W";
            break;
        case Log::Critical:
            sig = "C";
            break;
        case Log::Fatal:
            sig = "F";
            break;
        default:
            break;
    }
    console::printf(foreground, console::plain, "[%s] %-15s", dts.c_str(), category);
    console::printf(console::black, background, " %s ", sig);
    console::printf(console::plain, console::plain, "  ");
    console::vprintf(foreground, console::plain, fmt, args);
}

Context::Context() : logger("onnxtest") {
    // Get basic directories
    appDir = stdc::system::application_directory();
    defaultPluginDir = appDir.parent_path() / _TSTR("lib") / _TSTR("plugins") / _TSTR("dsinfer");

    // Set default plugin directories
    env.addPluginPath("com.diffsinger.InferenceDriver",
                      defaultPluginDir / _TSTR("inferencedrivers"));
}
