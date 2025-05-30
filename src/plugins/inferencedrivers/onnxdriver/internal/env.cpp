#include "env.h"

#include <memory>
#include <utility>

#include <stdcorelib/strings.h>
#include <stdcorelib/library.h>

#include <onnxruntime_cxx_api.h>

#include "onnxdriver_logger.h"

namespace fs = std::filesystem;

namespace dsinfer::onnxdriver {

    static Env *g_env = nullptr;

    class Env::Impl {
    public:
        bool load(const fs::path &path, ExecutionProvider ep, std::string *errorMessage) {
            onnxdriver_log().info("Env - Loading onnx environment");

            stdc::Library dylib;

            /**
             *  1. Load Ort shared library and create handle
             */
            onnxdriver_log().debug("Env - Loading ORT shared library from %1", path);
#ifdef _WIN32
            auto orgLibPath = stdc::Library::setLibraryPath(path.parent_path());
#endif
            if (!dylib.open(path, stdc::Library::ResolveAllSymbolsHint)) {
                std::string msg = stdc::formatN("Load library failed: %1 [%2]", dylib.lastError(), path);
                onnxdriver_log().critical("Env - %1", msg);
                if (errorMessage) {
                    *errorMessage = std::move(msg);
                }
                return false;
            }
#ifdef _WIN32
            stdc::Library::setLibraryPath(orgLibPath);
#endif

            /**
             *  2. Get Ort API getter handle
             */
            onnxdriver_log().debug("Env - Getting ORT API handle");
            auto handle = (OrtApiBase * (ORT_API_CALL *) ()) dylib.resolve("OrtGetApiBase");
            if (!handle) {
                std::string msg = stdc::formatN("Failed to get API handle: %1 [%2]", dylib.lastError(), path);
                onnxdriver_log().critical("Env - %1", msg);
                if (errorMessage) {
                    *errorMessage = std::move(msg);
                }
                return false;
            }

            /**
             *  3. Check Ort API
             */
            onnxdriver_log().debug("Env - ORT_API_VERSION is %1", ORT_API_VERSION);
            auto apiBase = handle();
            auto api = apiBase->GetApi(ORT_API_VERSION);
            if (!api) {
                std::string msg = stdc::formatN("%1: failed to get API instance");
                onnxdriver_log().critical("Env - %1", msg);
                if (errorMessage) {
                    *errorMessage = std::move(msg);
                }
                return false;
            }
            onnxdriver_log().debug("Env - ORT library version is %1", apiBase->GetVersionString());

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

            onnxdriver_log().info("Env - Load successful");
            return true;
        }

        stdc::Library lib;

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
        __stdc_impl_t;
        if (impl.loaded) {
            std::string msg = stdc::formatN(R"(Library "%1" has been loaded [%2])", impl.ortPath, path);
            onnxdriver_log().warning("Env - %1", msg);
            if (errorMessage) {
                *errorMessage = std::move(msg);
            }
            return false;
        }
        return impl.load(path, ep, errorMessage);
    }

    bool Env::isLoaded() const {
        __stdc_impl_t;
        return impl.loaded;
    }

    Env *Env::instance() {
        return g_env;
    }

    fs::path Env::runtimePath() const {
        __stdc_impl_t;
        return impl.ortPath;
    }

    ExecutionProvider Env::executionProvider() const {
        __stdc_impl_t;
        return impl.executionProvider;
    }

    int Env::deviceIndex() const {
        __stdc_impl_t;
        return impl.deviceIndex;
    }

    void Env::setDeviceIndex(int deviceIndex) {
        __stdc_impl_t;
        impl.deviceIndex = deviceIndex;
    }

    std::string Env::versionString() const {
        __stdc_impl_t;
        return impl.ortApiBase ? impl.ortApiBase->GetVersionString() : std::string();
    }

}