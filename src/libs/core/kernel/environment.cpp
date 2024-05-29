#include "environment.h"

#include <stdexcept>

#include <onnxruntime_cxx_api.h>

#include <loadso/library.h>
#include <loadso/system.h>

#include "dsinfer_pimpl.h"
#include "format.h"

namespace fs = std::filesystem;

namespace dsinfer {

    static Environment *g_env = nullptr;

    class Environment::Impl {
    public:
        bool load(const fs::path &path, ExecutionProvider ep, std::string *errorMessage) {
            LoadSO::Library tempLib;

            // 1. Load Ort shared library and create handle
#ifdef _WIN32
            auto orgLibPath = LoadSO::System::SetLibraryPath(path.parent_path());
#endif
            if (!tempLib.open(path, LoadSO::Library::ResolveAllSymbolsHint)) {
                *errorMessage =
                    formatTextN("%1: Load library failed: %2", path, tempLib.lastError());
                return false;
            }
#ifdef _WIN32
            LoadSO::System::SetLibraryPath(orgLibPath);
#endif

            // 2. Get Ort API getter handle
            auto addr = tempLib.resolve("OrtGetApiBase");
            if (!addr) {
                *errorMessage =
                    formatTextN("%1: Get api handle failed: %2", path, tempLib.lastError());
                return false;
            }

            // 3. Check Ort API
            auto handle = (OrtApiBase * (ORT_API_CALL *) ()) addr;
            auto apiBase = handle();
            auto api = apiBase->GetApi(ORT_API_VERSION);
            if (!api) {
                *errorMessage = formatTextN("%1: Failed to get api instance");
                return false;
            }

            // Successfully get Ort API.
            Ort::InitApi(api);

            std::swap(lib, tempLib);

            loaded = true;
            libPath = path;
            executionProvider = ep;

            ortApiBase = apiBase;
            ortApi = api;
            return true;
        }

        LoadSO::Library lib;

        // Metadata
        bool loaded = false;
        fs::path libPath;
        ExecutionProvider executionProvider = EP_CPU;

        // Library data
        void *hLibrary = nullptr;
        const OrtApi *ortApi = nullptr;
        const OrtApiBase *ortApiBase = nullptr;
    };

    Environment::Environment() : _impl(std::make_unique<Impl>()) {
        g_env = this;
    }

    Environment::~Environment() {
        g_env = nullptr;
    }

    bool Environment::load(const fs::path &path, ExecutionProvider ep, std::string *errorMessage) {
        __impl_t;
        if (impl.loaded) {
            *errorMessage = formatTextN("%1: Library \"%2\" has been loaded", path, impl.libPath);
            return false;
        }
        return impl.load(path, ep, errorMessage);
    }

    bool Environment::isLoaded() const {
        __impl_t;
        return impl.loaded;
    }

    Environment *Environment::instance() {
        return g_env;
    }

    fs::path Environment::libraryPath() const {
        __impl_t;
        return impl.libPath;
    }

    ExecutionProvider Environment::executionProvider() const {
        __impl_t;
        return impl.executionProvider;
    }

    std::string Environment::versionString() const {
        __impl_t;
        return impl.ortApiBase ? impl.ortApiBase->GetVersionString() : std::string();
    }

}