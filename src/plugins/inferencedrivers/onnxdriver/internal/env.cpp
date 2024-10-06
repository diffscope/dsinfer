#include "env.h"

#include <stdexcept>

#ifndef ORT_API_MANUAL_INIT
#define ORT_API_MANUAL_INIT
#endif

#include <onnxruntime_cxx_api.h>

#include <dsinfer/sharedlibrary.h>

#include <dsinfer/format.h>

namespace fs = std::filesystem;

namespace dsinfer {
namespace onnxdriver {

    static Env *g_env = nullptr;

    class Env::Impl {
    public:
        bool load(const fs::path &path, ExecutionProvider ep, std::string *errorMessage) {
            SharedLibrary tempLib;

            // 1. Load Ort shared library and create handle
#ifdef _WIN32
            //auto orgLibPath = LoadSO::System::SetLibraryPath(path.parent_path());
#endif
            if (!tempLib.open(path, SharedLibrary::ResolveAllSymbolsHint)) {
                *errorMessage =
                    formatTextN("%1: Load library failed: %2", path, tempLib.lastError());
                return false;
            }
#ifdef _WIN32
            //LoadSO::System::SetLibraryPath(orgLibPath);
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

    bool Env::load(const fs::path &path, ExecutionProvider ep,
                           std::string *errorMessage) {
        auto &impl = *_impl;
        if (impl.loaded) {
            *errorMessage =
                formatTextN("%1: Library \"%2\" has been loaded", path, impl.ortPath);
            return false;
        }
        return impl.load(path, ep, errorMessage);
    }

    bool Env::isLoaded() const {
        auto &impl = *_impl;
        return impl.loaded;
    }

    Env *Env::instance() {
        return g_env;
    }

    fs::path Env::runtimePath() const {
        auto &impl = *_impl;
        return impl.ortPath;
    }

    ExecutionProvider Env::executionProvider() const {
        auto &impl = *_impl;
        return impl.executionProvider;
    }

    int Env::deviceIndex() const {
        auto &impl = *_impl;
        return impl.deviceIndex;
    }

    void Env::setDeviceIndex(int deviceIndex) {
        auto &impl = *_impl;
        impl.deviceIndex = deviceIndex;
    }

    std::string Env::versionString() const {
        auto &impl = *_impl;
        return impl.ortApiBase ? impl.ortApiBase->GetVersionString() : std::string();
    }
}
}