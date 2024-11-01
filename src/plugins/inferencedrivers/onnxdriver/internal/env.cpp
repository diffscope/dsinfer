#include "env.h"

#include <utility>

#include <dsinfer/sharedlibrary.h>
#include <dsinfer/format.h>

#include <onnxruntime_cxx_api.h>

namespace fs = std::filesystem;

namespace dsinfer::onnxdriver {

    static Env *g_env = nullptr;

    class Env::Impl {
    public:
        bool load(const fs::path &path, ExecutionProvider ep, std::string *errorMessage) {
            SharedLibrary dylib;

            /**
             *  1. Load Ort shared library and create handle
             */
#ifdef _WIN32
            auto orgLibPath = SharedLibrary::setLibraryPath(path.parent_path());
#endif
            if (!dylib.open(path, SharedLibrary::ResolveAllSymbolsHint)) {
                *errorMessage = formatTextN("%1: load library failed: %2", path, dylib.lastError());
                return false;
            }
#ifdef _WIN32
            SharedLibrary::setLibraryPath(orgLibPath);
#endif

            /**
             *  2. Get Ort API getter handle
             */
            auto handle = (OrtApiBase * (ORT_API_CALL *) ()) dylib.resolve("OrtGetApiBase");
            if (!handle) {
                *errorMessage =
                    formatTextN("%1: failed to get API handle: %2", path, dylib.lastError());
                return false;
            }

            /**
             *  3. Check Ort API
             */
            auto apiBase = handle();
            auto api = apiBase->GetApi(ORT_API_VERSION);
            if (!api) {
                *errorMessage = formatTextN("%1: failed to get API instance");
                return false;
            }

            /**
             *  4. Successfully get Ort API
             */
            Ort::InitApi(api);

            std::swap(lib, dylib);
            loaded = true;
            ortPath = path;
            executionProvider = ep;
            ortApiBase = apiBase;
            ortApi = api;
            return true;
        }

        SharedLibrary lib;

        // Metadata
        bool loaded = false;
        fs::path ortPath;
        ExecutionProvider executionProvider = EP_CPU;
        int deviceIndex = 0;

        // Library data
        void *hLibrary = nullptr;
        const OrtApi *ortApi = nullptr;
        const OrtApiBase *ortApiBase = nullptr;
    };

    Env::Env() : _impl(std::make_unique<Impl>()) {
        g_env = this;
    }

    Env::~Env() {
        g_env = nullptr;
    }

    bool Env::load(const fs::path &path, ExecutionProvider ep, std::string *errorMessage) {
        __dsinfer_impl_t;
        if (impl.loaded) {
            *errorMessage = formatTextN(R"(%1: library "%2" has been loaded)", path, impl.ortPath);
            return false;
        }
        return impl.load(path, ep, errorMessage);
    }

    bool Env::isLoaded() const {
        __dsinfer_impl_t;
        return impl.loaded;
    }

    Env *Env::instance() {
        return g_env;
    }

    fs::path Env::runtimePath() const {
        __dsinfer_impl_t;
        return impl.ortPath;
    }

    ExecutionProvider Env::executionProvider() const {
        __dsinfer_impl_t;
        return impl.executionProvider;
    }

    int Env::deviceIndex() const {
        __dsinfer_impl_t;
        return impl.deviceIndex;
    }

    void Env::setDeviceIndex(int deviceIndex) {
        __dsinfer_impl_t;
        impl.deviceIndex = deviceIndex;
    }

    std::string Env::versionString() const {
        __dsinfer_impl_t;
        return impl.ortApiBase ? impl.ortApiBase->GetVersionString() : std::string();
    }

}