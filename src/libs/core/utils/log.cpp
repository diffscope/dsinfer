#include "log.h"

#ifdef _WIN32
#  include <windows.h>
#endif

#include <mutex>
#include <cstdio>

namespace dsinfer {

    static void log_default_callback(int level, const char *category, const char *fmt,
                                     va_list args) {
        // TODO
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
        va_list args;
        va_start(args, fmt);
        m_callback(level, category, fmt, args);
        va_end(args);
    }

    class PrintScopeGuard {
    public:
        static std::mutex &global_mtx() {
            static std::mutex _instance;
            return _instance;
        }

        explicit PrintScopeGuard(ConsoleOutput::Color color, bool light)
            : needReset(color != ConsoleOutput::NoColor) {
            global_mtx().lock();
#ifdef _WIN32
            _codepage = ::GetConsoleOutputCP();
            ::SetConsoleOutputCP(CP_UTF8);

            if (color != ConsoleOutput::NoColor) {
                WORD winColor = light ? FOREGROUND_INTENSITY : 0;
                switch (color) {
                    case ConsoleOutput::Red:
                        winColor |= FOREGROUND_RED;
                        break;
                    case ConsoleOutput::Green:
                        winColor |= FOREGROUND_GREEN;
                        break;
                    case ConsoleOutput::Yellow:
                        winColor |= FOREGROUND_RED | FOREGROUND_GREEN;
                        break;
                    case ConsoleOutput::White:
                        winColor |= FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
                    default:
                        break;
                }
                _hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
                GetConsoleScreenBufferInfo(_hConsole, &_csbi);
                SetConsoleTextAttribute(_hConsole, winColor);
            }
#else
            if (color != ConsoleOutput::NoColor) {
                // ANSI escape code to set text color to red
                const char *colorStr;
                switch (color) {
                    case ConsoleOutput::Red:
                        colorStr = light ? "\033[91m" : "\033[31m";
                        break;
                    case ConsoleOutput::Green:
                        colorStr = light ? "\033[92m" : "\033[32m";
                        break;
                    case ConsoleOutput::Yellow:
                        colorStr = light ? "\033[93m" : "\033[33m";
                        break;
                    case ConsoleOutput::White:
                        colorStr = light ? "\033[97m" : "\033[37m";
                        break;
                    default:
                        break;
                }
                printf("%s", colorStr);
            }
#endif
        }

        ~PrintScopeGuard() {
#ifdef _WIN32
            ::SetConsoleOutputCP(_codepage);

            if (needReset) {
                SetConsoleTextAttribute(_hConsole, _csbi.wAttributes);
            }
#else
            if (needReset) {
                // ANSI escape code to reset text color to default
                const char *resetColor = "\033[0m";
                printf("%s", resetColor);
            }
#endif
            global_mtx().unlock();
        }

    private:
        bool needReset;
#ifdef _WIN32
        UINT _codepage;
        HANDLE _hConsole;
        CONSOLE_SCREEN_BUFFER_INFO _csbi;
#endif
    };

    int ConsoleOutput::printf(Color color, bool light, const char *fmt, ...) {
        PrintScopeGuard _guard(color, light);

        va_list args;
        va_start(args, fmt);
        int res = std::vprintf(fmt, args);
        va_end(args);
        return res;
    }

    int ConsoleOutput::vprintf(Color color, bool light, const char *fmt, va_list args) {
        PrintScopeGuard _guard(color, light);
        return std::vprintf(fmt, args);
    }

}