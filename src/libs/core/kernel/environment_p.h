#ifndef ENVIRONMENT_P_H
#define ENVIRONMENT_P_H

#include <map>
#include <array>

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
        std::vector<ContributeRegistry *> registries;
        std::map<std::string, ContributeRegistry *> regSpecMap;

        std::vector<std::filesystem::path> libraryPaths;
        mutable std::shared_mutex env_mtx;
    };

}

#endif // ENVIRONMENT_P_H
