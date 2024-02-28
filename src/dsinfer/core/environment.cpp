#include <dsinfer/dsinfer_global.h>
#include <dsinfer/dsinfer_capi.h>
#include <dsinfer/ort_library.h>

#include <onnxruntime_cxx_api.h>
#include <syscmdline/system.h>

#include "environment.h"


namespace dsinfer {
    constexpr DSINFER_ExecutionProvider kDefaultEP = EP_CPU;

    Environment::Environment() : m_isInitialized(false), m_ep(kDefaultEP), m_env(nullptr) {}

    Environment &Environment::getInstance() {
        static Environment instance;
        return instance;
    }

    Status Environment::init(const std::string &path, DSINFER_ExecutionProvider ep) {
        if (m_isInitialized) {
            return {ET_LoadError, EC_EnvAlreadyInitialized, "The environment is already initialized"};
        }
        m_ep = ep;
        auto statusDllLoad = loadOrtLibrary(path);
        if (statusDllLoad.code() != EC_Success) {
            return statusDllLoad;
        }
        auto ortApi = Ort::GetApi();

        auto status = Ort::Status(ortApi.CreateEnv(ORT_LOGGING_LEVEL_WARNING, "dsinfer", &m_env));

        if (status) {
            auto errCode = status.GetErrorCode();
            auto errMsg = status.GetErrorMessage();
            if (!status.IsOK()) {
                return {ET_LoadError, EC_EnvInitializeFailed, errMsg};
            }
        }

        m_isInitialized = true;

        return {ET_LoadError, EC_Success, ""};
    }

    DSINFER_ExecutionProvider Environment::getExecutionProvider() const {
        return m_ep;
    }

    OrtEnv *Environment::getOrtEnv() {
        return m_env;
    }
    Environment::~Environment() {
        if (isOrtLoaded()) {
            auto ortApi = Ort::GetApi();
            ortApi.ReleaseEnv(m_env);
        }
    }
} // namespace dsinfer


DSINFER_EXPORT DSINFER_Status *dsinfer_init(const char *path, DSINFER_ExecutionProvider ep) {
    auto status = dsEnv.init(path, ep);
    return status.release();
}
