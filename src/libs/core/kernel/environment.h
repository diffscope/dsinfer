#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <map>
#include <filesystem>

#include <dsinferCore/dsinfercoreglobal.h>
#include <dsinferCore/dsinfer_common.h>

#define dsEnv (dsinfer::Environment::instance())

namespace dsinfer {

    class DSINFER_CORE_EXPORT Environment {
    public:
        Environment();
        ~Environment();

        static Environment *instance();

    public:
        bool load(const std::filesystem::path &path, ExecutionProvider ep,
                  std::string *errorMessage);
        bool isLoaded() const;

        std::filesystem::path runtimePath() const;
        ExecutionProvider executionProvider() const;
        std::string versionString() const;

    protected:
        class Impl;
        std::unique_ptr<Impl> _impl;
    };

}

#endif // ENVIRONMENT_H
