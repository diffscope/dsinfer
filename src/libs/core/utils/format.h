#ifndef FORMAT_H
#define FORMAT_H

#include <string>
#include <typeinfo>
#include <vector>
#include <filesystem>
#include <sstream>

#include <dsinfer/dsinferglobal.h>

namespace dsinfer {

    DSINFER_EXPORT std::string wideToUtf8(const wchar_t *s, int size = -1);

    DSINFER_EXPORT std::wstring utf8ToWide(const char *s, int size = -1);

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
            return anyToString(t.string());
        } else if constexpr (std::is_same_v<T2, std::wstring>) {
            return wideToUtf8(t.data(), int(t.size()));
        } else if constexpr (std::is_same_v<T2, wchar_t *>) {
            return wideToUtf8(t);
        } else {
            return std::string(t);
        }
    }

    DSINFER_EXPORT std::string formatText(const std::string &format,
                                          const std::vector<std::string> &args);

    template <typename... Args>
    auto formatTextN(const std::string &format, Args &&...args) {
        return formatText(format, {anyToString(std::forward<decltype(args)>(args))...});
    }

    inline std::filesystem::path pathFromString(const std::string &s) {
#ifdef _WIN32
        return utf8ToWide(s.data(), int(s.size()));
#else
        return s;
#endif
    }


}

#endif // FORMAT_H
