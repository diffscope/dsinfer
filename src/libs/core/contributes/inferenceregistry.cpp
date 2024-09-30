#include "inferenceregistry.h"

#include <unordered_map>
#include <list>

#include "inferencespec_p.h"
#include "contributeregistry_p.h"
#include "environment_p.h"
#include "format.h"
#include "inferencedriverplugin.h"

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

    std::vector<InferenceSpec *>
        InferenceRegistry::findInferences(const ContributeIdentifier &identifier) const {
        __dsinfer_impl_t;
        std::vector<InferenceSpec *> res;
        auto temp = impl.findContributes(identifier);
        res.reserve(res.size());
        for (const auto &item : std::as_const(temp)) {
            res.push_back(static_cast<InferenceSpec *>(item));
        }
        return res;
    }

    std::vector<InferenceSpec *> InferenceRegistry::inferences() const {
        __dsinfer_impl_t;
        std::shared_lock<std::shared_mutex> lock(impl.env_mtx());
        std::vector<InferenceSpec *> res;
        res.reserve(impl.contributes.size());
        for (const auto &item : impl.contributes) {
            res.push_back(static_cast<InferenceSpec *>(item));
        }
        return res;
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

    InferenceDriver *InferenceRegistry::createDriver(const char *key) const {
        auto plugin = env()->plugin<InferenceDriverPlugin>(key);
        if (!plugin) {
            return nullptr;
        }
        return plugin->create();
    }

    std::string InferenceRegistry::specKey() const {
        static std::string _key("inferences");
        return _key;
    }

    ContributeSpec *InferenceRegistry::parseSpec(const std::filesystem::path &basePath,
                                                 const JsonValue &config, Error *error) const {
        __dsinfer_impl_t;
        if (!config.isObject()) {
            *error = {
                Error::InvalidFormat,
                R"(invalid inference specification)",
            };
            return nullptr;
        }
        auto spec = new InferenceSpec();
        if (!spec->_impl->read(basePath, config.toObject(), error)) {
            delete spec;
            return nullptr;
        }
        return spec;
    }

    bool InferenceRegistry::loadSpec(ContributeSpec *spec, ContributeSpec::State state,
                                     Error *error) {
        __dsinfer_impl_t;
        switch (state) {
            case ContributeSpec::Initialized: {
                auto inferenceSpec = static_cast<InferenceSpec *>(spec);
                // Search interpreter
                auto interp =
                    env()->plugin<InferenceInterpreter>(inferenceSpec->className().data());
                if (!interp) {
                    *error = {
                        Error::FeatureNotSupported,
                        formatTextN(R"(inference interpreter "%1" not found)",
                                    inferenceSpec->className()),
                    };
                    return false;
                }
                // Check schema and configuration
                std::string errMsg;
                if (!interp->validate(inferenceSpec, &errMsg)) {
                    *error = {
                        Error::InvalidFormat,
                        formatTextN(R"(inference "%1" validate failed: %2)",
                                    inferenceSpec->className(), errMsg),
                    };
                    return false;
                }
                auto spec_d = static_cast<InferenceSpec::Impl *>(inferenceSpec->_impl.get());
                spec_d->interp = interp;
                return ContributeRegistry::loadSpec(spec, state, error);
            }
            case ContributeSpec::Ready:
            case ContributeSpec::Finished: {
                return true;
            }
            case ContributeSpec::Deleted: {
                return ContributeRegistry::loadSpec(spec, state, error);
            }
            default:
                break;
        }
        return false;
    }

    InferenceRegistry::InferenceRegistry(Environment *env) : ContributeRegistry(*new Impl(env)) {
    }

}