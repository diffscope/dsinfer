#include <onnxruntime_cxx_api.h>

#include "onnxdriver_common.h"
#include "onnxdriver_logger.h"
#include "env.h"
#include "executionprovider.h"
#include "session.h"
#include "sessionsystem.h"
#include "sessionimage.h"

namespace dsinfer::onnxdriver {

    Ort::Session createOrtSession(const Ort::Env &env, const std::filesystem::path &modelPath, bool preferCpu, std::string *errorMessage = nullptr);

    static inline Log::Category ort_log();

    void loggingFuncOrt(void* param, OrtLoggingLevel severity, const char* category, const char* logid, const char* code_location, const char* message);

    SessionImage::SessionImage(std::filesystem::path path)
        : path(std::move(path)), count(1),
          env(ORT_LOGGING_LEVEL_WARNING, "flowonnx", loggingFuncOrt, nullptr),
          session(nullptr) {
    }

    int SessionImage::ref() {
        count++;
        onnxdriver_log().debug("SessionImage [%1] - ref(), now ref count = %2", path.filename(), count);
        return count;
    }

    int SessionImage::deref() {
        count--;
        auto filename = path.filename();
        onnxdriver_log().debug("SessionImage [%1] - deref(), now ref count = %2", filename, count);
        if (count == 0) {
            onnxdriver_log().debug("SessionImage [%1] - removing from session image map", filename);
            if (!SessionSystem::instance()->removeImage(path)) {
                onnxdriver_log().critical("SessionImage [%1] - removing failed: image does not exist in SessionSystem!", filename);
            }
            onnxdriver_log().debug("SessionImage [%1] - delete", filename);
            delete this;
            return 0;
        }
        return count;
    }

    bool SessionImage::init(bool preferCpu, std::string *errorMessage) {
        session = createOrtSession(env, path, preferCpu, errorMessage);
        if (session) {
            SessionSystem::instance()->addImage(path, this, true);

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

            return true;
        }
        return false;
    }

    SessionImage *SessionImage::create(const std::filesystem::path &onnxPath, bool preferCpu, std::string *errorMessage) {
        auto filename = onnxPath.filename();
        onnxdriver_log().debug("SessionImage [%1] - create", filename);
        auto imagePtr = new SessionImage(onnxPath);
        bool ok = imagePtr->init(preferCpu, errorMessage);
        if (!ok) {
            delete imagePtr;
            onnxdriver_log().critical("SessionImage [%1] - create failed", filename);
            return nullptr;
        }
        onnxdriver_log().debug("SessionImage [%1] - created successfully", filename);
        return imagePtr;
    }

    Ort::Session createOrtSession(const Ort::Env &env, const std::filesystem::path &modelPath, bool preferCpu, std::string *errorMessage) {
        try {
            Ort::SessionOptions sessOpt;

            auto flowonnxEnv = Env::instance();
            auto ep = flowonnxEnv->executionProvider();
            auto deviceIndex = flowonnxEnv->deviceIndex();

            std::string initEPErrorMsg;
            if (!preferCpu) {
                switch (ep) {
                    case EP_DirectML: {
                        if (!initDirectML(sessOpt, deviceIndex, &initEPErrorMsg)) {
                            // log warning: "Could not initialize DirectML: {initEPErrorMsg}, falling back to CPU."
                            onnxdriver_log().warning("Could not initialize DirectML: %1, falling back to CPU.", initEPErrorMsg);
                        } else {
                            onnxdriver_log().info("Use DirectML. Device index: %1", deviceIndex);
                        }
                        break;
                    }
                    case EP_CUDA: {
                        if (!initCUDA(sessOpt, deviceIndex, &initEPErrorMsg)) {
                            // log warning: "Could not initialize CUDA: {initEPErrorMsg}, falling back to CPU."
                            onnxdriver_log().warning("Could not initialize CUDA: %1, falling back to CPU.", initEPErrorMsg);
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

#ifdef _WIN32
            auto pathStr = modelPath.wstring();
#else
            auto pathStr = modelPath.string();
#endif
            return Ort::Session{ env, pathStr.c_str(), sessOpt };
        } catch (const Ort::Exception &e) {
            if (errorMessage) {
                *errorMessage = e.what();
            }
        }
        return Ort::Session{ nullptr };
    }

    static inline Log::Category ort_log() {
        return Log::Category("onnxruntime");
    }

    void loggingFuncOrt(void* param, OrtLoggingLevel severity, const char* category, const char* logid, const char* code_location, const char* message) {
        std::string logMessage = std::string("[") + code_location + "] " + message;
        switch (severity) {
            case ORT_LOGGING_LEVEL_VERBOSE:
                ort_log().verbose(logMessage);
                break;
            case ORT_LOGGING_LEVEL_INFO:
                ort_log().info(logMessage);
                break;
            case ORT_LOGGING_LEVEL_WARNING:
                ort_log().warning(logMessage);
                break;
            case ORT_LOGGING_LEVEL_ERROR:
                ort_log().critical(logMessage);
                break;
            case ORT_LOGGING_LEVEL_FATAL:
                ort_log().fatal(logMessage);
                break;
        }
    }
}