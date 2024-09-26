#include "inferenceregistry.h"

#include "contributeregistry_p.h"
#include "environment_p.h"

namespace dsinfer {

    class InferenceRegistry::Impl : public ContributeRegistry::Impl {
    public:
        explicit Impl(Environment *env) : ContributeRegistry::Impl(InferenceSpec::Inference, env) {
        }

        ~Impl() {
            delete driver;
        }

        InferenceDriver *driver = nullptr;
    };

    InferenceRegistry::~InferenceRegistry() = default;

    InferenceSpec *InferenceRegistry::findInference(const std::string &id,
                                                    const VersionNumber &version) const {
        __dsinfer_impl_t;
        std::shared_lock<std::shared_mutex> lock(impl.env_mtx());
        // TODO
        return nullptr;
    }

    std::vector<InferenceSpec *> InferenceRegistry::findInferences(const std::string &id) const {
        __dsinfer_impl_t;
        std::shared_lock<std::shared_mutex> lock(impl.env_mtx());
        // TODO
        return {};
    }

    std::vector<InferenceSpec *> InferenceRegistry::inferences() const {
        __dsinfer_impl_t;
        std::shared_lock<std::shared_mutex> lock(impl.env_mtx());
        // TODO
        return {};
    }

    InferenceDriver *InferenceRegistry::driver() const {
        __dsinfer_impl_t;
        std::unique_lock<std::shared_mutex> lock(impl.env_mtx());
        return impl.driver;
    }

    InferenceDriver *InferenceRegistry::takeDriver() {
        __dsinfer_impl_t;
        std::unique_lock<std::shared_mutex> lock(impl.env_mtx());
        auto org = impl.driver;
        impl.driver = nullptr;
        return org;
    }

    void InferenceRegistry::setDriver(InferenceDriver *driver) {
        __dsinfer_impl_t;
        std::unique_lock<std::shared_mutex> lock(impl.env_mtx());
        delete impl.driver;
        impl.driver = driver;
    }

    std::string InferenceRegistry::specKey() const {
        static std::string _key("inferences");
        return _key;
    }

    ContributeSpec *InferenceRegistry::parseSpec(const std::filesystem::path &basePath,
                                                 const JsonObject &config, Error *error) const {
        __dsinfer_impl_t;
        // TODO
        return nullptr;
    }

    void InferenceRegistry::unload(ContributeSpec *spec) {
        __dsinfer_impl_t;
        // TODO
    }

    InferenceRegistry::InferenceRegistry(Environment *env) : ContributeRegistry(*new Impl(env)) {
    }

}