#include "inferenceregistry.h"

#include <list>
#include <mutex>
#include <unordered_map>

#include <stdcorelib/strings.h>

#include "inferencespec_p.h"
#include "contributeregistry_p.h"
#include "inferencedriverplugin.h"
#include "inferenceinterpreterplugin.h"

namespace dsinfer {

    class InferenceRegistry::Impl : public ContributeRegistry::Impl {
    public:
        explicit Impl(Environment *env) : ContributeRegistry::Impl(InferenceSpec::Inference, env) {
        }

        ~Impl() {
            delete driver;
        }

        InferenceDriver *driver = nullptr;
        std::unordered_map<std::string, InferenceInterpreter *> interpreters;
    };

    InferenceRegistry::~InferenceRegistry() = default;

    bool InferenceRegistry::setup(const char *key, const JsonValue &args, Error *error) {
        __stdc_impl_t;
        auto plugin = impl.env->plugin<InferenceDriverPlugin>(key);
        if (!plugin) {
            *error = {
                Error::FileNotFound,
                stdc::formatN(R"(driver "%1" not found)", key),
            };
            return false;
        }
        auto driver = plugin->create();
        if (!driver->initialize(args, error)) {
            delete driver;
            return false;
        }
        std::unique_lock<std::shared_mutex> lock(impl.env_mtx());
        impl.driver = driver;
        return true;
    }

    std::vector<InferenceSpec *>
        InferenceRegistry::findInferences(const ContributeIdentifier &identifier) const {
        __stdc_impl_t;
        std::vector<InferenceSpec *> res;
        auto temp = impl.findContributes(identifier);
        res.reserve(res.size());
        for (const auto &item : std::as_const(temp)) {
            res.push_back(static_cast<InferenceSpec *>(item));
        }
        return res;
    }

    std::vector<InferenceSpec *> InferenceRegistry::inferences() const {
        __stdc_impl_t;
        std::shared_lock<std::shared_mutex> lock(impl.env_mtx());
        std::vector<InferenceSpec *> res;
        res.reserve(impl.contributes.size());
        for (const auto &item : impl.contributes) {
            res.push_back(static_cast<InferenceSpec *>(item));
        }
        return res;
    }

    InferenceDriver *InferenceRegistry::driver() const {
        __stdc_impl_t;
        std::unique_lock<std::shared_mutex> lock(impl.env_mtx());
        return impl.driver;
    }

    std::string InferenceRegistry::specKey() const {
        static std::string _key("inferences");
        return _key;
    }

    ContributeSpec *InferenceRegistry::parseSpec(const std::filesystem::path &basePath,
                                                 const JsonValue &config, Error *error) const {
        __stdc_impl_t;
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
        __stdc_impl_t;
        switch (state) {
            case ContributeSpec::Initialized: {
                auto inferenceSpec = static_cast<InferenceSpec *>(spec);
                const auto &key = inferenceSpec->className();
                InferenceInterpreter *interp = nullptr;

                // Search interpreter cache
                if (auto it = impl.interpreters.find(key); it != impl.interpreters.end()) {
                    interp = it->second;
                } else {
                    // Search interpreter in file system
                    auto plugin = env()->plugin<InferenceInterpreterPlugin>(
                        inferenceSpec->className().c_str());
                    if (!plugin) {
                        *error = {
                            Error::FeatureNotSupported,
                            stdc::formatN(
                                R"(required interpreter "%1" of inference "%2" not found)",
                                inferenceSpec->className(), inferenceSpec->id()),
                        };
                        return false;
                    }
                    interp = plugin->create();
                    impl.interpreters[key] = interp;
                }

                // Check api level
                if (interp->apiLevel() < inferenceSpec->apiLevel()) {
                    *error = {
                        Error::FeatureNotSupported,
                        stdc::formatN(
                            R"(required interpreter "%1" of api level %2 doesn't support inference "%3" of api level %4)",
                            inferenceSpec->className(), interp->apiLevel(), inferenceSpec->id(),
                            inferenceSpec->apiLevel()),
                    };
                    return false;
                }

                // Check schema and configuration
                std::string errMsg;
                if (!interp->validate(inferenceSpec, &errMsg)) {
                    *error = {
                        Error::InvalidFormat,
                        stdc::formatN(R"(inference "%1" validate failed: %2)", inferenceSpec->id(),
                                      errMsg),
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