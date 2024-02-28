#ifndef DSINFER_ENVIRONMENT_H
#define DSINFER_ENVIRONMENT_H

#include <onnxruntime_cxx_api.h>
#include <dsinfer/dsinfer_capi.h>
#include <dsinfer/dsinfer_cxxapi.h>

#define dsEnv (dsinfer::Environment::getInstance())

namespace dsinfer {

    class Environment final {
    public:
        static Environment &getInstance();

        Status init(const std::string &path, DSINFER_ExecutionProvider ep);

        [[nodiscard]] DSINFER_ExecutionProvider getExecutionProvider() const;

        [[nodiscard]] OrtEnv *getOrtEnv();

    public:
        Environment(const Environment &) = delete;
        Environment(Environment &&) = delete;
        Environment & operator=(const Environment &) = delete;
    private:
        Environment();
        ~Environment();

        bool m_isInitialized;

        DSINFER_ExecutionProvider m_ep;

        OrtEnv *m_env;
    };

} // namespace dsinfer

#endif // DSINFER_ENVIRONMENT_H
