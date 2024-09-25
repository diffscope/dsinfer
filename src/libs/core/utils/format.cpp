#include "format.h"

#ifdef _WIN32
#  include <windows.h>
#endif

namespace dsinfer {

    std::string wideToUtf8(const std::wstring &utf16Str) {
#ifdef _WIN32
        if (utf16Str.empty()) {
            return {};
        }
        const auto utf16Length = static_cast<int>(utf16Str.size());
        if (utf16Length >= (std::numeric_limits<int>::max)()) {
            return {};
        }
        const int utf8Length = ::WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, utf16Str.data(),
                                                     utf16Length, nullptr, 0, nullptr, nullptr);
        if (utf8Length <= 0) {
            return {};
        }
        std::string utf8Str;
        utf8Str.resize(utf8Length);
        ::WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, utf16Str.data(), utf16Length,
                              utf8Str.data(), utf8Length, nullptr, nullptr);
        return utf8Str;
#else
        return std::filesystem::path(utf16Str).string();
#endif
    }

    /*!
        Returns the UTF-8 multi-byte string converted from wide string.
    */
    std::wstring utf8ToWide(const std::string &utf8Str) {
#ifdef _WIN32
        if (utf8Str.empty()) {
            return {};
        }
        const auto utf8Length = static_cast<int>(utf8Str.size());
        if (utf8Length >= (std::numeric_limits<int>::max)()) {
            return {};
        }
        const int utf16Length = ::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8Str.data(),
                                                      utf8Length, nullptr, 0);
        if (utf16Length <= 0) {
            return {};
        }
        std::wstring utf16Str;
        utf16Str.resize(utf16Length);
        ::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8Str.data(), utf8Length,
                              utf16Str.data(), utf16Length);
        return utf16Str;
#else
        return std::filesystem::path(utf8Str).wstring();
#endif
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

}