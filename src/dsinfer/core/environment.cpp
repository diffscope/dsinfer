#include "environment.h"

#include <stdexcept>

#include <onnxruntime_cxx_api.h>

#include <loadso/library.h>
#include <loadso/system.h>

namespace fs = std::filesystem;

namespace dsinfer {

    static Environment *g_env = nullptr;

    class Environment::Impl {
    public:
        void load(const fs::path &path, ExecutionProvider ep) {
            // 1. Load Ort shared library and create handle

#ifdef _WIN32
            LoadSO::System::SetLibraryPath(path.parent_path());
#endif
            if (!lib.open(path)) {
                auto msg = std::string("Load library failed: ") +
                           LoadSO::System::WideToMulti(lib.lastError());
                throw std::runtime_error(msg);
            }
#ifdef _WIN32
            LoadSO::System::SetLibraryPath({});
#endif

            // 2. Get Ort API getter handle
            auto addr = lib.entry("OrtGetApiBase");
            if (!addr) {
                lib.close();

                auto msg = std::string("Get api handle failed: ") +
                           LoadSO::System::WideToMulti(lib.lastError());
                throw std::runtime_error(msg);
            }

            // 3. Check Ort API
            auto handle = (OrtApiBase * (ORT_API_CALL *) ()) addr;
            auto apiBase = handle();
            auto api = apiBase->GetApi(ORT_API_VERSION);
            if (!api) {
                lib.close();

                auto msg = std::string("Failed to get OrtApi.");
                throw std::runtime_error(msg);
            }

            // Successfully get Ort API.
            Ort::InitApi(api);

            loaded = true;
            libPath = path;
            executionProvider = ep;

            ortApiBase = apiBase;
            ortApi = api;
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

    void Environment::load(const fs::path &path, ExecutionProvider ep) {
        if (_impl->loaded)
            return;
        _impl->load(path, ep);
    }

    bool Environment::isLoaded() const {
        return _impl->loaded;
    }

    Environment *Environment::instance() {
        return g_env;
    }

    fs::path Environment::libraryPath() const {
        return _impl->libPath;
    }

    ExecutionProvider Environment::executionProvider() const {
        return _impl->executionProvider;
    }

    std::string Environment::versionString() const {
        return _impl->ortApiBase ? _impl->ortApiBase->GetVersionString() : std::string();
    }

}