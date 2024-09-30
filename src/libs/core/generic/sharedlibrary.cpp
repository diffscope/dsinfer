#include "sharedlibrary.h"

#include <cctype>

#ifdef _WIN32
#  include <Windows.h>
// 12345
#  include <Shlwapi.h>
#else
#  include <dlfcn.h>
#  include <limits.h>
#  include <string.h>
#endif

namespace fs = std::filesystem;

namespace dsinfer {

    class SharedLibrary::Impl {
    public:
        void *hDll = nullptr;
        fs::path path;

        virtual ~Impl();

        static inline int nativeLoadHints(int loadHints);
        static std::string sysErrorMessage(bool nativeLanguage);

        bool open(int hints = 0);
        bool close();
        void *resolve(const char *name) const;
    };

#ifdef _WIN32

    static constexpr const DWORD g_EnglishLangId = MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);

    static std::wstring winErrorMessage(uint32_t error, bool nativeLanguage = true) {
        std::wstring rc;
        wchar_t *lpMsgBuf;

        const DWORD len =
            ::FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                                 FORMAT_MESSAGE_IGNORE_INSERTS,
                             NULL, error, nativeLanguage ? 0 : g_EnglishLangId,
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

        return rc;
    }

    static std::string wide2utf8(const std::wstring &str) {
        int len = ::WideCharToMultiByte(CP_UTF8, 0, str.data(), (int) str.size(), nullptr, 0,
                                        nullptr, nullptr);
        auto buf = new char[len + 1];
        ::WideCharToMultiByte(CP_UTF8, 0, str.data(), (int) str.size(), buf, len, nullptr, nullptr);
        buf[len] = '\0';

        std::string res(buf);
        delete[] buf;
        return res;
    }

#endif

    SharedLibrary::Impl::~Impl() {
        std::ignore = close();
    }

    inline int SharedLibrary::Impl::nativeLoadHints(int loadHints) {
#ifdef _WIN32
        return 0;
#else
        int dlFlags = 0;
        if (loadHints & SharedLibrary::ResolveAllSymbolsHint) {
            dlFlags |= RTLD_NOW;
        } else {
            dlFlags |= RTLD_LAZY;
        }
        if (loadHints & ExportExternalSymbolsHint) {
            dlFlags |= RTLD_GLOBAL;
        }
#  if !defined(Q_OS_CYGWIN)
        else {
            dlFlags |= RTLD_LOCAL;
        }
#  endif
#  if defined(RTLD_DEEPBIND)
        if (loadHints & DeepBindHint)
            dlFlags |= RTLD_DEEPBIND;
#  endif
        return dlFlags;
#endif
    }

    std::string SharedLibrary::Impl::sysErrorMessage(bool nativeLanguage) {
#ifdef _WIN32
        return wide2utf8(winErrorMessage(::GetLastError(), nativeLanguage));
#else
        auto err = dlerror();
        if (err) {
            return err;
        }
        return {};
#endif
    }

    bool SharedLibrary::Impl::open(int hints) {
        auto absPath = fs::absolute(path);

        auto handle =
#ifdef _WIN32
            ::LoadLibraryW(absPath.c_str())
#else
            dlopen(absPath.c_str(), Impl::nativeLoadHints(hints))
#endif
            ;
        if (!handle) {
            return false;
        }

#ifdef _WIN32
        if (hints & PreventUnloadHint) {
            // prevent the unloading of this component
            HMODULE hmod;
            ::GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_PIN |
                                     GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
                                 reinterpret_cast<const wchar_t *>(handle), &hmod);
        }
#endif

        hDll = handle;
        return true;
    }

    bool SharedLibrary::Impl::close() {
        if (!hDll) {
            return true;
        }

        if (!
#ifdef _WIN32
            ::FreeLibrary(reinterpret_cast<HMODULE>(hDll))
#else
            (dlclose(hDll) == 0)
#endif
        ) {
            return false;
        }

        hDll = nullptr;
        return true;
    }

    void *SharedLibrary::Impl::resolve(const char *name) const {
        if (!hDll) {
            return nullptr;
        }

        auto addr =
#ifdef _WIN32
            ::GetProcAddress(reinterpret_cast<HMODULE>(hDll), name)
#else
            dlsym(hDll, name)
#endif
            ;
        return reinterpret_cast<void *>(addr);
    }

    SharedLibrary::SharedLibrary() : _impl(new Impl()) {
    }

    SharedLibrary::~SharedLibrary() = default;

    SharedLibrary::SharedLibrary(SharedLibrary &&other) noexcept {
        std::swap(_impl, other._impl);
    }

    SharedLibrary &SharedLibrary::operator=(SharedLibrary &&other) noexcept {
        if (this == &other)
            return *this;
        std::swap(_impl, other._impl);
        return *this;
    }

    bool SharedLibrary::open(const fs::path &path, int hints) {
        _impl->path = path;
        if (_impl->open(hints)) {
            _impl->path = fs::canonical(fs::absolute(path));
            return true;
        }
        _impl->path.clear();
        return false;
    }

    bool SharedLibrary::close() {
        if (_impl->close()) {
            _impl->path.clear();
            return true;
        }
        return false;
    }

    bool SharedLibrary::isOpen() const {
        return _impl->hDll != nullptr;
    }

    fs::path SharedLibrary::path() const {
        return _impl->path;
    }

    void *SharedLibrary::handle() const {
        return _impl->hDll;
    }

    void *SharedLibrary::resolve(const char *name) const {
        return _impl->resolve(name);
    }

    std::string SharedLibrary::lastError() const {
        return _impl->sysErrorMessage(false);
    }

#if !defined(_WIN32) && !defined(__APPLE__)
    static bool checkVersionSuffix(const std::string_view &suffix) {
        size_t start = 0;
        while (start < suffix.size()) {
            size_t dotPos = suffix.find('.', start);
            std::string_view part;
            if (dotPos == std::string::npos) {
                part = suffix.substr(start);
                start = suffix.size();
            } else {
                part = suffix.substr(start, dotPos - start);
                start = dotPos + 1;
            }
            if (!std::all_of(part.begin(), part.end(), ::isdigit)) {
                return false;
            }
        }
        return true;
    }
#endif

    bool SharedLibrary::isLibrary(const fs::path &path) {
#if defined(_WIN32)
        auto fileName = path.wstring();
        return fileName.size() >= 4 &&
               std::equal(fileName.end() - 4, fileName.end(), L".dll", [](wchar_t a, wchar_t b) {
                   return ::tolower(a) == ::tolower(b); //
               });
#elif defined(__APPLE__)
        auto fileName = path.string();
        return fileName.size() >= 6 &&
               std::equal(fileName.end() - 6, fileName.end(), L".dylib", [](char a, char b) {
                   return ::tolower(a) == ::tolower(b); //
               });
#else
        size_t soPos;
        if (fileName.size() >= 3 && (soPos = fileName.rfind(".so")) != std::string::npos) {
            // 检查 .so 后是否有版本号部分
            std::string_view suffix = std::string_view(fileName).substr(soPos + 3);
            if (suffix.empty()) {
                return true; // 仅有 .so，无版本号
            }
            return checkVersionSuffix(suffix); // 确保后缀全为数字
        }
        return false;
#endif
    }

}