#include "format.h"

#ifdef _WIN32
#  include <windows.h>
#endif

namespace fs = std::filesystem;

namespace dsinfer {

    std::string wideToUtf8(const wchar_t *s, int size) {
#ifdef _WIN32
        if (size < 0) {
            size = (int) wcslen(s);
        }
        if (size == 0) {
            return {};
        }
        const int utf8Length = ::WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, s, size,
                                                     nullptr, 0, nullptr, nullptr);
        if (utf8Length <= 0) {
            return {};
        }
        std::string utf8Str;
        utf8Str.resize(utf8Length);
        ::WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, s, size, utf8Str.data(), utf8Length,
                              nullptr, nullptr);
        return utf8Str;
#else
        return std::filesystem::path(std::wstring(s, size)).string();
#endif
    }

    /*!
        Returns the UTF-8 multi-byte string converted from wide string.
    */
    std::wstring utf8ToWide(const char *s, int size) {
#ifdef _WIN32
        if (size < 0) {
            size = (int) strlen(s);
        }
        if (size == 0) {
            return {};
        }
        const int utf16Length =
            ::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, s, size, nullptr, 0);
        if (utf16Length <= 0) {
            return {};
        }
        std::wstring utf16Str;
        utf16Str.resize(utf16Length);
        ::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, s, size, utf16Str.data(), utf16Length);
        return utf16Str;
#else
        return std::filesystem::path(std::string(s, size)).wstring();
#endif
    }

    std::string ansiToUtf8(const char *s, int size) {
        if (size < 0) {
            size = (int) strlen(s);
        }
        if (size == 0) {
            return {};
        }
        const int utf16Length =
            ::MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS, s, size, nullptr, 0);
        if (utf16Length <= 0) {
            return {};
        }
        std::wstring utf16Str;
        utf16Str.resize(utf16Length);
        ::MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS, s, size, utf16Str.data(), utf16Length);
        return wideToUtf8(utf16Str.data(), int(utf16Str.size()));
    }

    std::filesystem::path cleanPath(const std::filesystem::path &path) {
        fs::path result;
        for (const auto &part : path) {
            if (part == _TSTR("..")) {
                if (!result.empty() && result.filename() != _TSTR("..")) {
                    result = result.parent_path();
                } else {
                    result /= part;
                }
            } else if (part != _TSTR(".")) {
                result /= part;
            }
        }
        return result;
    }

    DSINFER_EXPORT std::string normalizePathSeparators(const std::string &path, bool native) {
        std::string res = path;
#if _WIN32
        if (native) {
            std::replace(res.begin(), res.end(), '/', '\\');

        } else {
            std::replace(res.begin(), res.end(), '\\', '/');
        }
#else
        (void) native;
        std::replace(res.begin(), res.end(), '\\', '/');
#endif
        return res;
    }

    std::string formatText(const std::string &format, const std::vector<std::string> &args) {
        std::string result = format;
        for (size_t i = 0; i < args.size(); i++) {
            std::string placeholder = "%" + std::to_string(i + 1);
            size_t pos = result.find(placeholder);
            while (pos != std::string::npos) {
                result.replace(pos, placeholder.length(), args[i]);
                pos = result.find(placeholder, pos + args[i].size());
            }
        }
        return result;
    }

    std::vector<std::string> split(const std::string &s, const std::string &delimiter) {
        std::vector<std::string> tokens;
        std::string::size_type start = 0;
        std::string::size_type end = s.find(delimiter);
        while (end != std::string::npos) {
            tokens.push_back(s.substr(start, end - start));
            start = end + delimiter.size();
            end = s.find(delimiter, start);
        }
        tokens.push_back(s.substr(start));
        return tokens;
    }

    std::string join(const std::vector<std::string> &v, const std::string &delimiter) {
        if (v.empty())
            return {};

        std::string res;
        for (int i = 0; i < v.size() - 1; ++i) {
            res.append(v[i]);
            res.append(delimiter);
        }
        res.append(v.back());
        return res;
    }

}