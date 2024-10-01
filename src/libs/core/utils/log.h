#ifndef DSINFER_LOG_H
#define DSINFER_LOG_H

#include <cstdlib>

#include <dsinfer/format.h>

namespace dsinfer {

    class DSINFER_EXPORT Log {
    public:
        enum Level {
            Trace = 1,
            Debug,
            Verbose,
            Information,
            Warning,
            Critical,
            Fatal,
        };

        using Callback = void (*)(int, const char *, const char *, va_list);

        static int level();
        static void setLevel(int level);

        static Callback callback();
        static void setCallback(Callback callback);

        template <class... Args>
        static inline void print(int level, const char *category, const std::string &format,
                                 Args &&...args) {
            std::ignore = printf(level, category, "%s\n", formatTextN(format, args...).c_str());
        }

        static void printf(int level, const char *category, const char *fmt, ...)
            DSINFER_PRINTF_FORMAT(3, 4);

    public:
        struct Category {
            inline Category(const char *category) : category(category) {
            }

            template <class... Args>
            inline void trace(const std::string &format, Args &&...args) {
                print(Log::Trace, category, format, args...);
            }

            template <class... Args>
            inline void debug(const std::string &format, Args &&...args) {
                print(Debug, category, format, args...);
            }

            template <class... Args>
            inline void verbose(const std::string &format, Args &&...args) {
                print(Verbose, category, format, args...);
            }

            template <class... Args>
            inline void info(const std::string &format, Args &&...args) {
                print(Information, category, format, args...);
            }

            template <class... Args>
            inline void warning(const std::string &format, Args &&...args) {
                print(Warning, category, format, args...);
            }

            template <class... Args>
            inline void critical(const std::string &format, Args &&...args) {
                print(Critical, category, format, args...);
            }

            template <class... Args>
            inline void fatal(const std::string &format, Args &&...args) {
                print(Fatal, category, format, args...);
                std::abort();
            }

        private:
            const char *category;
        };
    };

    class DSINFER_EXPORT ConsoleOutput {
    public:
        enum Color {
            NoColor,
            Red,
            Green,
            Yellow,
            White,
        };

        static int printf(Color color, bool light, const char *fmt, ...)
            DSINFER_PRINTF_FORMAT(3, 4);

        static int vprintf(Color color, bool light, const char *fmt, va_list args);
    };

}

#endif // DSINFER_LOG_H
