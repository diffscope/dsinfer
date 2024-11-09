#include "sessionimage.h"

#include <onnxruntime_cxx_api.h>

#include "onnxdriver_common.h"
#include "onnxdriver_logger.h"
#include "executionprovider.h"
#include "env.h"

namespace dsinfer::onnxdriver {

    static Ort::Session createOrtSession(const Ort::Env &ortEnv,
                                         const std::filesystem::path &modelPath, bool preferCpu,
                                         std::string *errorMessage) {
        try {
            Ort::SessionOptions sessOpt;

            auto env = Env::instance();
            auto ep = env->executionProvider();
            auto deviceIndex = env->deviceIndex();

            std::string initEPErrorMsg;
            if (!preferCpu) {
                switch (ep) {
                    case EP_DirectML: {
                        if (!initDirectML(sessOpt, deviceIndex, &initEPErrorMsg)) {
                            // log warning: "Could not initialize DirectML: {initEPErrorMsg},
                            // falling back to CPU."
                            onnxdriver_log().warning(
                                "Could not initialize DirectML: %1, falling back to CPU.",
                                initEPErrorMsg);
                        } else {
                            onnxdriver_log().info("Use DirectML. Device index: %1", deviceIndex);
                        }
                        break;
                    }
                    case EP_CUDA: {
                        if (!initCUDA(sessOpt, deviceIndex, &initEPErrorMsg)) {
                            // log warning: "Could not initialize CUDA: {initEPErrorMsg}, falling
                            // back to CPU."
                            onnxdriver_log().warning(
                                "Could not initialize CUDA: %1, falling back to CPU.",
                                initEPErrorMsg);
                        } else {
                            onnxdriver_log().info("Use CUDA. Device index: %1", deviceIndex);
                        }
                        break;
                    }
                    default: {
                        // log info: "Use CPU."
                        onnxdriver_log().info("Use CPU.");
                        break;
                    }
                }
            } else {
                onnxdriver_log().info("The model prefers to use CPU. [%1]", modelPath.filename());
            }
            return Ort::Session{ortEnv, std::filesystem::path::string_type(modelPath).c_str(),
                                sessOpt};
        } catch (const Ort::Exception &e) {
            if (errorMessage) {
                *errorMessage = e.what();
            }
        }
        return Ort::Session{nullptr};
    }

    static void loggingFuncOrt(void *param, OrtLoggingLevel severity, const char *category,
                               const char *logid, const char *code_location, const char *message) {
        int log_level = Log::Information;
        switch (severity) {
            case ORT_LOGGING_LEVEL_VERBOSE:
                log_level = Log::Verbose;
                break;
            case ORT_LOGGING_LEVEL_WARNING:
                log_level = Log::Warning;
                break;
            case ORT_LOGGING_LEVEL_ERROR:
                log_level = Log::Critical;
                break;
            case ORT_LOGGING_LEVEL_FATAL:
                log_level = Log::Fatal;
                break;
            default:
                break;
        }
        Log::Category("onnxruntime").log(log_level, "[%1] %2", code_location, message);
    }

    SessionImage::SessionImage()
        : env(ORT_LOGGING_LEVEL_WARNING, "flowonnx", loggingFuncOrt, nullptr), session(nullptr) {
    }

    SessionImage::~SessionImage() = default;

    bool SessionImage::open(const std::filesystem::path &onnxPath, int hints,
                            std::string *errorMessage) {
        auto filename = onnxPath.filename();
        onnxdriver_log().debug("SessionImage [%1] - creating", filename);

        session = createOrtSession(env, onnxPath, hints & SH_PreferCPUHint, errorMessage);
        if (!session) {
            onnxdriver_log().critical("SessionImage [%1] - create failed", filename);
            return false;
        }
        Ort::AllocatorWithDefaultOptions allocator;

        auto inputCount = session.GetInputCount();
        inputNames.reserve(inputCount);
        for (size_t i = 0; i < inputCount; ++i) {
            inputNames.emplace_back(session.GetInputNameAllocated(i, allocator).get());
        }

        auto outputCount = session.GetOutputCount();
        outputNames.reserve(outputCount);
        for (size_t i = 0; i < outputCount; ++i) {
            outputNames.emplace_back(session.GetOutputNameAllocated(i, allocator).get());
        }
        onnxdriver_log().debug("SessionImage [%1] - created successfully", filename);
        return true;
    }

}