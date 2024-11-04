#include "onnxdriver.h"

#include "onnxsession.h"
#include "onnxtask.h"
#include "onnxcontext.h"

#include "env.h"

namespace dsinfer {

    class OnnxDriver::Impl {
    public:
        explicit Impl(const std::filesystem::path &runtimePath) : runtimePath(runtimePath) {
        }

        ~Impl() {
            if (initialized) {
                delete shared_env;
            }
        }

        std::filesystem::path runtimePath;
        bool initialized = false;

        static inline onnxdriver::Env *shared_env = nullptr;
    };

    OnnxDriver::OnnxDriver(const std::filesystem::path &runtimePath)
        : _impl(std::make_unique<Impl>(runtimePath)) {
    }

    OnnxDriver::~OnnxDriver() {
    }

    bool OnnxDriver::initialize(const JsonValue &args, Error *error) {
        __stdc_impl_t;
        if (impl.shared_env) {
            if (error) {
                *error = {
                    Error::FileDuplicated,
                    "onnx runtime has been initialized by another instance",
                };
            }
            return false;
        }

        // Parse args
        onnxdriver::ExecutionProvider ep = onnxdriver::EP_CPU;
        int deviceIndex = 0;
        {
            auto obj = args.toObject();

            // ep
            if (auto it = obj.find("ep"); it != obj.end()) {
                if (it->second.isString()) {
                    auto epString = it->second.toString();
                    if (epString == "dml") {
                        ep = onnxdriver::EP_DirectML;
                    } else if (epString == "cuda") {
                        ep = onnxdriver::EP_CUDA;
                    } else if (epString == "coreml") {
                        ep = onnxdriver::EP_CoreML;
                    } else {
                        ep = onnxdriver::EP_CPU;
                    }
                } else {
                    ep = onnxdriver::EP_CPU;
                }
            }

            // device index
            if (auto it = obj.find("deviceIndex"); it != obj.end()) {
                if (it->second.isInt()) {
                    deviceIndex = static_cast<int>(it->second.toInt());
                } else {
                    deviceIndex = 0;
                }
            }
        }

        auto dllPath = impl.runtimePath /
#if defined(_WIN32)
                       L"onnxruntime.dll"
#elif defined(__APPLE__)
                       "libonnxruntime.dylib"
#else
                       "libonnxruntime.so"
#endif
            ;

        // Load
        auto env = new onnxdriver::Env();
        if (std::string errorMessage; !env->load(dllPath, ep, &errorMessage)) {
            if (error) {
                *error = Error(Error::LibraryNotFound, errorMessage);
            }
            delete env;
            return false;
        }
        env->setDeviceIndex(deviceIndex);

        impl.initialized = true;
        impl.shared_env = env;
        return true;
    }

    InferenceSession *OnnxDriver::createSession() {
        return new OnnxSession();
    }

    InferenceTask *OnnxDriver::createTask() {
        return new OnnxTask();
    }

    InferenceContext *OnnxDriver::createContext() {
        return new OnnxContext();
    }

}