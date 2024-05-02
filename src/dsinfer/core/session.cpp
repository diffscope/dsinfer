#include <memory>

#include <onnxruntime_cxx_api.h>

#ifdef DSINFER_ENABLE_DIRECTML
#  include <dml_provider_factory.h>
#endif

#include <dsinfer/dsinfer_common.h>
#include <dsinfer/environment.h>
#include "session.h"

namespace fs = std::filesystem;

namespace dsinfer {

    struct ModelConfig {
        fs::path path;
        ModelType type = MT_Unknown;
    };

    static ModelConfig parseModelConfig(const fs::path &configPath);
    static Ort::Status initCUDA(Ort::SessionOptions &options, int deviceIndex);
    static Ort::Status initDirectML(Ort::SessionOptions &options, int deviceIndex);
    static std::vector<std::string> getModelInputNames(const Ort::Session &session);
    static std::vector<std::string> getModelOutputNames(const Ort::Session &session);

    class Session::Impl {
    public:
        Impl()
            : m_env(ORT_LOGGING_LEVEL_WARNING, "dsinfer"), m_runOptions(), m_session(nullptr) {
        }

        void load(const fs::path &path, int deviceIndex) {
            unsetTerminate();

            auto modelConfig = parseModelConfig(path);
            if (modelConfig.type == MT_Unknown) {
                // TODO: error handling
                return;
            }

            try {
                Ort::SessionOptions options;
                auto ep = dsEnv->executionProvider();
                switch (ep) {
                    case EP_CUDA:
                        initCUDA(options, deviceIndex);
                        break;
                    case EP_DirectML:
                        initDirectML(options, deviceIndex);
                        break;
                    default:
                        break;
                }

                auto modelPathString =
#ifdef _WIN32
                    modelConfig.path.wstring();
#else
                    modelConfig.path.string();
#endif
                m_session = Ort::Session(m_env, modelPathString.c_str(), options);

                inputNames = getModelInputNames(m_session);
                outputNames = getModelOutputNames(m_session);
                type = modelConfig.type;
                loaded = true;
            } catch (const Ort::Exception &ortException) {
                // TODO: error handling
            }
        }

        void free() {
            terminate();
            {
                // After swapping, when `tmp` leaves the scope, the session is destructed.
                Ort::Session tmp(nullptr);
                std::swap(m_session, tmp);
            }
            unsetTerminate();

            inputNames = {};
            outputNames = {};
            type = MT_Unknown;
        }

        void terminate() {
            m_runOptions.SetTerminate();
        }

        void unsetTerminate() {
            m_runOptions.UnsetTerminate();
        }

        bool loaded = false;
        ModelType type = MT_Unknown;
        std::vector<std::string> inputNames{}, outputNames{};

    private:
        Ort::Env m_env;
        Ort::RunOptions m_runOptions;
        Ort::Session m_session;
    };

    Session::Session() : _impl(std::make_unique<Impl>()) {
    }

    Session::~Session() {
        free();
    }

    void Session::load(const fs::path &path, int deviceIndex) {
        if (_impl->loaded) {
            return;
        }
        _impl->load(path, deviceIndex);
    }

    void Session::free() {
        if (!_impl->loaded) {
            return;
        }
        _impl->free();
    }

    void Session::terminate() {
        if (!_impl->loaded) {
            return;
        }
        _impl->terminate();
    }

    void Session::unsetTerminate() {
        if (!_impl->loaded) {
            return;
        }
        _impl->unsetTerminate();
    }

    ModelType Session::type() const {
        return _impl->type;
    }

    std::vector<std::string> Session::inputNames() const {
        return _impl->inputNames;
    }

    std::vector<std::string> Session::outputNames() const {
        return _impl->outputNames;
    }

    static ModelConfig parseModelConfig(const fs::path &configPath) {
        // TODO: parse model config JSON
        return {};
    }

    static Ort::Status initCUDA(Ort::SessionOptions &options, int deviceIndex) {
#ifdef DSINFER_ENABLE_CUDA
        if (!options) {
            return {"SessionOptions must not be nullptr!", ORT_EP_FAIL};
        }

        if (deviceIndex < 0) {
            return {"GPU device index must be a non-negative integer!", ORT_EP_FAIL};
        }

        const OrtApi &ortApi = Ort::GetApi();

        OrtCUDAProviderOptionsV2 *cudaOptionsPtr = nullptr;
        Ort::Status createStatus(ortApi.CreateCUDAProviderOptions(&cudaOptionsPtr));

        // Currently, ORT C++ API does not have a wrapper for CUDAProviderOptionsV2.
        // Let the smart pointer take ownership of cudaOptionsPtr so it will be released when it
        // goes out of scope.
        std::unique_ptr<OrtCUDAProviderOptionsV2, decltype(ortApi.ReleaseCUDAProviderOptions)>
            cudaOptions(cudaOptionsPtr, ortApi.ReleaseCUDAProviderOptions);

        if (!createStatus.IsOK()) {
            return createStatus;
        }

        // The following block of code sets device_id
        {
            // Device ID from int to string
            auto cudaDeviceIdStr = std::to_string(deviceIndex);
            auto cudaDeviceIdCStr = cudaDeviceIdStr.c_str();

            constexpr int CUDA_OPTIONS_SIZE = 2;
            const char *cudaOptionsKeys[CUDA_OPTIONS_SIZE] = {"device_id",
                                                              "cudnn_conv_algo_search"};
            const char *cudaOptionsValues[CUDA_OPTIONS_SIZE] = {cudaDeviceIdCStr, "DEFAULT"};
            Ort::Status updateStatus(ortApi.UpdateCUDAProviderOptions(
                cudaOptions.get(), cudaOptionsKeys, cudaOptionsValues, CUDA_OPTIONS_SIZE));
            if (!updateStatus.IsOK()) {
                return updateStatus;
            }
        }

        return Ort::Status(
            ortApi.SessionOptionsAppendExecutionProvider_CUDA_V2(options, cudaOptions.get()));
#else
        return {"The library is not built with CUDA support.", ORT_EP_FAIL};
#endif
    }

    static Ort::Status initDirectML(Ort::SessionOptions &options, int deviceIndex) {
#ifdef DSINFER_ENABLE_DIRECTML
        if (!options) {
            return {"SessionOptions must not be nullptr!", ORT_EP_FAIL};
        }

        if (deviceIndex < 0) {
            return {"GPU device index must be a non-negative integer!", ORT_EP_FAIL};
        }

        const OrtDmlApi *ortDmlApi;
        Ort::Status getApiStatus((ortApi.GetExecutionProviderApi(
            "DML", ORT_API_VERSION, reinterpret_cast<const void **>(&ortDmlApi))));
        if (!getApiStatus.IsOK()) {
            // Failed to get DirectML API.
            return getApiStatus;
        }

        // Successfully get DirectML API
        options.DisableMemPattern();
        options.SetExecutionMode(ORT_SEQUENTIAL);

        return Ort::Status(
            ortDmlApi->SessionOptionsAppendExecutionProvider_DML(options, deviceIndex));
#else
        return {"The library is not built with DirectML support.", ORT_EP_FAIL};
#endif
    }

    static std::vector<std::string> getModelInputNames(const Ort::Session &session) {
        auto n = session.GetInputCount();
        std::vector<std::string> arr;
        arr.reserve(n);

        Ort::AllocatorWithDefaultOptions allocator;

        for (size_t i = 0; i < n; ++i) {
            auto ptr = session.GetInputNameAllocated(i, allocator);
            arr.emplace_back(ptr.get());
        }

        return arr;
    }

    static std::vector<std::string> getModelOutputNames(const Ort::Session &session) {
        auto n = session.GetOutputCount();
        std::vector<std::string> arr;
        arr.reserve(n);

        Ort::AllocatorWithDefaultOptions allocator;

        for (size_t i = 0; i < n; ++i) {
            auto ptr = session.GetOutputNameAllocated(i, allocator);
            arr.emplace_back(ptr.get());
        }

        return arr;
    }

}