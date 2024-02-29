#include <onnxruntime_cxx_api.h>

#include <dsinfer/dsinfer_capi.h>
#include <dsinfer/dsinfer_cxxapi.h>
#include <dsinfer/environment.h>

#include <syscmdline/system.h>

#include <iostream>

#ifdef _WIN32
#include <Windows.h>
#else
#include <dlfcn.h>
#endif

#include "ort_library.h"

#ifdef _WIN32
namespace {
    std::string winErrorMessage(uint32_t error, bool nativeLanguage) {
        std::wstring rc;
        wchar_t *lpMsgBuf;

        const DWORD len = ::FormatMessageW(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, error, nativeLanguage ? 0 : MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
            reinterpret_cast<LPWSTR>(&lpMsgBuf), 0, NULL);

        if (len) {
            // Remove tail line breaks
            if (lpMsgBuf[len - 1] == L'\n') {
                lpMsgBuf[len - 1] = L'\0';
                if (len > 2 && lpMsgBuf[len - 2] == L'\r') {
                    lpMsgBuf[len - 2] = L'\0';
                }
            }
            rc = std::wstring(lpMsgBuf, int(len));
            ::LocalFree(lpMsgBuf);
        } else {
            rc += L"unknown error";
        }

        return SysCmdLine::wideToUtf8(rc);
    }
} // namespace
#endif

namespace dsinfer {

    static void *g_ortLibHandle = nullptr;
    static const OrtApi *g_ortApi = nullptr;
    static const OrtApiBase *g_ortApiBase = nullptr;
    static bool g_isOrtLoaded = false;

    Status loadOrtLibrary(const std::string &path) {
        if (g_isOrtLoaded) {
            // The library should not be loaded multiple times.
            return {ET_LoadError, EC_OnnxRuntimeAlreadyLoaded, "ONNX Runtime is already loaded."};
        }

        // Load Ort dynamic library and create handle.
        auto handle =
#ifdef _WIN32
            ::LoadLibraryExW(SysCmdLine::utf8ToWide(path).c_str(), nullptr, LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
#else
            dlopen(path.c_str(), RTLD_NOW);
#endif

        if (!handle) {
            std::string errMsg =
#ifdef _WIN32
                std::string("LoadLibrary failed: ") + winErrorMessage(::GetLastError(), false);
#else
                std::string("dlopen failed: ") + dlerror();
#endif
            return {ET_LoadError, EC_OnnxRuntimeLoadFailed, errMsg};
        }

        // Successfully loaded ORT dll.

        auto addr = (OrtApiBase * (ORT_API_CALL *) ())
#ifdef _WIN32
            ::GetProcAddress(handle, "OrtGetApiBase");
#else
            dlsym(handle, "OrtGetApiBase");
#endif
        if (!addr) {
            std::string errMsg =
#ifdef _WIN32
                std::string("GetProcAddress failed: ") + winErrorMessage(::GetLastError(), false);
#else
                std::string("dlsym failed: ") + dlerror();
#endif
            return {ET_LoadError, EC_OnnxRuntimeLoadFailed, errMsg};
        }

        // Successfully got the address of OrtGetApiBase function.

        g_ortLibHandle = handle;

        g_ortApiBase = addr();

        g_ortApi = g_ortApiBase->GetApi(ORT_API_VERSION);

        if (!g_ortApi) {
            return {ET_LoadError, EC_OnnxRuntimeLoadFailed, "g_ortApiBase->GetApi failed."};
        }

        // Successfully got ORT API.
        Ort::InitApi(g_ortApi);

        g_isOrtLoaded = true;
        //std::cout << "ort load success! version: " << version() << '\n';
        return {ET_LoadError, EC_Success, ""};
    }

    bool isOrtLoaded() {
        return g_isOrtLoaded;
    }

    std::string getOrtVersionString() {
        return g_ortApiBase->GetVersionString();
    }
} // namespace dsinfer
