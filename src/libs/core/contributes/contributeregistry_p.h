#ifndef RESOURCEMANAGER_P_H
#define RESOURCEMANAGER_P_H

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

        inline std::shared_mutex &env_mtx() const {
            return static_cast<Environment::Impl *>(env->_impl.get())->env_mtx;
        }

        static void setSpecState(ContributeSpec *spec, ContributeSpec::State state);
    };

}

#endif // RESOURCEMANAGER_P_H
