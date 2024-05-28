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
        void load(const std::filesystem::path &path, ExecutionProvider ep);
        bool isLoaded() const;

        std::filesystem::path libraryPath() const;
        ExecutionProvider executionProvider() const;
        std::string versionString() const;

    protected:
        class Impl;
        std::unique_ptr<Impl> _impl;

        Environment(const Environment &) = delete;
        Environment &operator=(const Environment &) = delete;
    };

}

#endif // ENVIRONMENT_H
