#include "log.h"

#include <stdcorelib/console.h>

namespace dsinfer {

    static void log_default_callback(int level, const char *category, const char *fmt,
                                     va_list args) {
        std::ignore = category;

        using stdc::Console;

        Console::Color color;
        if (level <= Log::Information) {
            color = Console::Default;
        } else if (level <= Log::Warning) {
            color = Console::Yellow;
        } else {
            color = Console::Red;
        }
        Console::vprintf(color, Console::Default, fmt, args);
    }

    static int m_level = Log::Information;

    static Log::Callback m_callback = log_default_callback;

    int Log::level() {
        return m_level;
    }

    void Log::setLevel(int level) {
        m_level = level;
    }

    Log::Callback Log::callback() {
        return m_callback;
    }

    void Log::setCallback(Log::Callback callback) {
        m_callback = callback;
    }

    void Log::printf(int level, const char *category, const char *fmt, ...) {
        if (level < m_level) {
            return;
        }
        va_list args;
        va_start(args, fmt);
        m_callback(level, category, fmt, args);
        va_end(args);
    }

    void Log::vprintf(int level, const char *category, const char *fmt, va_list args) {
        if (level < m_level) {
            return;
        }
        m_callback(level, category, fmt, args);
    }

}