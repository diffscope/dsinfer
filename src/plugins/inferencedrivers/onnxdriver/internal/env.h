#ifndef DSINFER_ONNXDRIVER_ENV_H
#define DSINFER_ONNXDRIVER_ENV_H

#include <memory>
#include <filesystem>

#include "onnxdriver_common.h"

namespace dsinfer::onnxdriver {

    class Env {
    public:
        Env();
        ~Env();

        Env(const Env &) = delete;
        Env &operator=(const Env &) = delete;

        static Env *instance();
    public:
        bool load(const std::filesystem::path &path, ExecutionProvider ep,
                  std::string *errorMessage);
        bool isLoaded() const;

        std::filesystem::path runtimePath() const;
        ExecutionProvider executionProvider() const;

        int deviceIndex() const;
        void setDeviceIndex(int deviceIndex);
        
        std::string versionString() const;

    protected:
        class Impl;
        std::unique_ptr<Impl> _impl;
    };

}

#endif // DSINFER_ONNXDRIVER_ENV_H
