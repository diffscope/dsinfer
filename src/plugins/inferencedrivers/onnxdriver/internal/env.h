#ifndef ONNXDRIVER_ENVIRONMENT_H
#define ONNXDRIVER_ENVIRONMENT_H

#include <map>
#include <filesystem>

#include "onnxdriver_common.h"

namespace dsinfer {
    namespace onnxdriver {

        class Env {
        public:
            Env();
            ~Env();

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
}

#endif // ONNXDRIVER_ENVIRONMENT_H
