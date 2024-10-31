#ifndef DSINFER_CONTRIBUTEREGISTRY_P_H
#define DSINFER_CONTRIBUTEREGISTRY_P_H

#include <shared_mutex>

#include <dsinfer/contributeregistry.h>
#include <dsinfer/private/environment_p.h>

namespace dsinfer {

    class ContributeRegistry::Impl {
    public:
        explicit Impl(int type, Environment *env) : type(type), env(env) {
        }
        virtual ~Impl() = default;

    public:
        int type;
        Environment *env;

        std::list<ContributeSpec *> contributes;
        std::unordered_map<
            std::string,
            std::unordered_map<VersionNumber,
                               std::unordered_map<std::string, decltype(contributes)::iterator>>>
            indexes;

        inline std::shared_mutex &env_mtx() const {
            return static_cast<Environment::Impl *>(env->_impl.get())->env_mtx;
        }

        std::vector<ContributeSpec *> findContributes(const ContributeIdentifier &identifier) const;
    };

}

#endif // DSINFER_CONTRIBUTEREGISTRY_P_H
