#ifndef DSINFER_ENVIRONMENT_H
#define DSINFER_ENVIRONMENT_H

#include <filesystem>

#include <dsinfer/dsinfer_common.h>

#define dsEnv (dsinfer::Environment::instance())

namespace dsinfer {

    class Environment {
    public:
        Environment();
        ~Environment();

        static Environment *instance();

    public:
        void load(const std::filesystem::path &path, ExecutionProvider ep);
        bool isLoaded() const;

        std::filesystem::path libraryPath() const;
        ExecutionProvider executionProvider() const;
        std::string versionString() const;

    protected:
        class Impl;
        std::unique_ptr<Impl> _impl;

        DSINFER_DISABLE_COPY(Environment)
    };

} // namespace dsinfer

#endif // DSINFER_ENVIRONMENT_H
