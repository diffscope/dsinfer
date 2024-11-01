#include "plugin.h"

#ifdef _WIN32
#  include <Windows.h>
#else
#  include <dlfcn.h>
#  include <unistd.h>
#  include <limits.h>
#endif

namespace dsinfer {

#ifdef _WIN32
    static std::wstring winGetFullModuleFileName(HMODULE hModule) {
        // Use stack buffer for the first try
        wchar_t stackBuf[MAX_PATH + 1];

        // Call
        wchar_t *buf = stackBuf;
        auto size = ::GetModuleFileNameW(hModule, buf, MAX_PATH);
        if (size == 0) {
            return {};
        }
        if (size > MAX_PATH) {
            buf = new wchar_t[size + 1]; // The return size doesn't contain the terminating 0
            if (::GetModuleFileNameW(hModule, buf, size) == 0) {
                delete[] buf;
                return {};
            }
        }

        // Return
        std::wstring res(buf);
        if (buf != stackBuf) {
            delete[] buf;
        }
        return res;
    }
#endif

    Plugin::~Plugin() = default;

    std::filesystem::path Plugin::path() const {
        const auto addr = this;
#ifdef _WIN32
        HMODULE hModule = nullptr;
        if (!::GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                                      GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                                  (LPCWSTR) addr, &hModule)) {
            return {};
        }
        return winGetFullModuleFileName(hModule);
#else
        Dl_info dl_info;
        dladdr((void *) addr, &dl_info);
        auto buf = dl_info.dli_fname;
        return buf;
#endif
    }

}