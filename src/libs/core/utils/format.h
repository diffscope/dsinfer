#ifndef DSINFER_FORMAT_H
#define DSINFER_FORMAT_H

#include <string>
#include <vector>
#include <filesystem>
#include <sstream>
#include <string_view>

#include <dsinfer/dsinferglobal.h>

namespace dsinfer {

    DSINFER_EXPORT std::string wideToUtf8(const wchar_t *s, int size = -1);

    inline std::string wideToUtf8(const std::wstring &s) {
        return wideToUtf8(s.c_str(), int(s.size()));
    }

    DSINFER_EXPORT std::wstring utf8ToWide(const char *s, int size = -1);

    inline std::wstring utf8ToWide(const std::string &s) {
        return utf8ToWide(s.c_str(), int(s.size()));
    }

#ifdef _WIN32
    DSINFER_EXPORT std::string ansiToUtf8(const char *s, int size = -1);

    inline std::string ansiToUtf8(const std::string &s) {
        return ansiToUtf8(s.c_str(), int(s.size()));
    }
#endif

    inline std::filesystem::path pathFromString(const std::string &s) {
#ifdef _WIN32
        return utf8ToWide(s);
#else
        return s;
#endif
    }

    inline std::string pathToString(const std::filesystem::path &path) {
#ifdef _WIN32
        return wideToUtf8(path.wstring());
#else
        return path.string();
#endif
    }

    DSINFER_EXPORT std::filesystem::path cleanPath(const std::filesystem::path &path);

    DSINFER_EXPORT std::string normalizePathSeparators(const std::string &path,
                                                       bool native = false);

    template <class T>
    std::string anyToString(T &&t) {
        using T2 = std::decay_t<std::remove_cv_t<std::remove_reference_t<T>>>;
        if constexpr (std::is_same_v<T2, bool>) {
            return t ? "true" : "false";
        } else if constexpr (std::is_integral_v<T2>) {
            return std::to_string(t);
        } else if constexpr (std::is_floating_point_v<T2>) {
            std::ostringstream oss;
            oss << std::noshowpoint << t;
            return oss.str();
        } else if constexpr (std::is_same_v<T2, std::filesystem::path>) {
            return normalizePathSeparators(pathToString(t), true);
        } else if constexpr (std::is_same_v<T2, std::wstring>) {
            return wideToUtf8(t);
        } else if constexpr (std::is_same_v<T2, wchar_t *>) {
            return wideToUtf8(t);
        } else {
            return std::string(t);
        }
    }

    DSINFER_EXPORT std::string formatText(const std::string &format,
                                          const std::vector<std::string> &args);

    template <class... Args>
    auto formatTextN(const std::string &format, Args &&...args) {
        return formatText(format, {anyToString(std::forward<decltype(args)>(args))...});
    }

    DSINFER_EXPORT std::vector<std::string_view> split(const std::string_view &s,
                                                       const std::string_view &delimiter);

    DSINFER_EXPORT std::string join(const std::vector<std::string> &v,
                                    const std::string_view &delimiter);

}

#endif // DSINFER_FORMAT_H
