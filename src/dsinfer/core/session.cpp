#include <memory>

#include <onnxruntime_cxx_api.h>

#ifdef DSINFER_ENABLE_DIRECTML
#  include <dml_provider_factory.h>
#endif

#include <dsinfer/dsinfer_common.h>
#include <dsinfer/environment.h>

#include "session.h"
#include "session_p.h"

namespace dsinfer {

    static Ort::Status initCUDA(Ort::SessionOptions &options, int deviceIndex);
    static Ort::Status initDirectML(Ort::SessionOptions &options, int deviceIndex);
    static std::vector<std::string> getModelInputNames(const Ort::Session &session);
    static std::vector<std::string> getModelOutputNames(const Ort::Session &session);

    SessionPrivate::SessionPrivate() :
          refCount(0),
          forceOnCpu(false),
          m_env(ORT_LOGGING_LEVEL_WARNING, "dsinfer"),
          m_session(nullptr) {}

    SessionPrivate::~SessionPrivate() {
        free();
    }

    bool SessionPrivate::load() {
        std::lock_guard<std::mutex> lock(mtx);

        if (refCount > 0) {
            ++refCount;
            return true;
        }

        try {
            Ort::SessionOptions options;
            if (!forceOnCpu) {
                auto ep = dsEnv->executionProvider();
                auto deviceIndex = dsEnv->deviceIndex();
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
            }

            auto onnxPathString =
#ifdef _WIN32
                onnxPath.wstring();
#else
                onnxPath.string();
#endif
            m_session = Ort::Session(m_env, onnxPathString.c_str(), options);

            inputNames = getModelInputNames(m_session);
            outputNames = getModelOutputNames(m_session);
            ++refCount;
            return true;
        } catch (const Ort::Exception &ortException) {
            // TODO: error handling
        }
        return false;
    }

    void SessionPrivate::free() {
        std::lock_guard<std::mutex> lock(mtx);

        if (refCount == 0) {
            return;
        }
        if (refCount > 1) {
            --refCount;
            return;
        }
        m_session = Ort::Session(nullptr);

        inputNames = {};
        outputNames = {};
        refCount = 0;
    }

    Session::Session(const std::filesystem::path &path, bool forceOnCpu) : d(std::make_unique<SessionPrivate>()) {
        d->onnxPath = path;
        d->forceOnCpu = forceOnCpu;
    }

    Session::Session(Session &&other) noexcept : d(std::move(other.d)) {
        other.d = nullptr;
    }

    Session &Session::operator=(Session &&other) noexcept {
        if (this != &other) {
            d = std::move(other.d);
            other.d = nullptr;
        }
        return *this;
    }

    Session::~Session() = default;

    std::vector<std::string> Session::inputNames() const {
        return d->inputNames;
    }

    std::vector<std::string> Session::outputNames() const {
        return d->outputNames;
    }

    size_t Session::useCount() const {
        return d->refCount;
    }

    bool Session::isLoaded() const {
        return d->m_session;
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