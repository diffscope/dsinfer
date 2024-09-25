#ifndef ENVIRONMENT_P_H
#define ENVIRONMENT_P_H

#include <dsinfer/environment.h>
#include <dsinfer/private/pluginfactory_p.h>

namespace dsinfer {

    class Environment::Impl : public PluginFactory::Impl {
    public:
        explicit Impl(Environment *decl);
        ~Impl();

    public:
        using Decl = Environment;

    public:
        ContributeRegistry *registries[2];
        std::vector<std::filesystem::path> libraryPaths;
        mutable std::shared_mutex env_mtx;
    };

}

#endif // ENVIRONMENT_P_H
