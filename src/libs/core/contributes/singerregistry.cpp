#include "singerregistry.h"
#include "contributeregistry_p.h"

#include "singerspec_p.h"
#include "inferenceregistry.h"
#include "format.h"

namespace dsinfer {

    class SingerRegistry::Impl : public ContributeRegistry::Impl {
    public:
        explicit Impl(Environment *env) : ContributeRegistry::Impl(ContributeSpec::Singer, env) {
        }

        std::list<SingerSpec *> singers;
        std::unordered_map<
            std::string,
            std::unordered_map<VersionNumber,
                               std::unordered_map<std::string, decltype(singers)::iterator>>>
            indexes;
    };

    SingerRegistry::~SingerRegistry() = default;

    std::vector<SingerSpec *>
        SingerRegistry::findSingers(const ContributeIdentifier &identifier) const {
        __dsinfer_impl_t;
        std::vector<SingerSpec *> res;
        auto temp = impl.findContributes(identifier);
        res.reserve(res.size());
        for (const auto &item : std::as_const(temp)) {
            res.push_back(static_cast<SingerSpec *>(item));
        }
        return res;
    }

    std::vector<SingerSpec *> SingerRegistry::singers() const {
        __dsinfer_impl_t;
        std::shared_lock<std::shared_mutex> lock(impl.env_mtx());
        return {impl.singers.begin(), impl.singers.end()};
    }

    std::string SingerRegistry::specKey() const {
        static std::string _key("singers");
        return _key;
    }

    ContributeSpec *SingerRegistry::parseSpec(const std::filesystem::path &basePath,
                                              const JsonValue &config, Error *error) const {
        if (!config.isObject()) {
            *error = {
                Error::InvalidFormat,
                R"(invalid inference specification)",
            };
            return nullptr;
        }
        auto spec = new SingerSpec();
        if (!spec->_impl->read(basePath, config.toObject(), error)) {
            delete spec;
            return nullptr;
        }
        return spec;
    }

    bool SingerRegistry::loadSpec(ContributeSpec *spec, ContributeSpec::State state, Error *error) {
        __dsinfer_impl_t;
        switch (state) {
            case ContributeSpec::Initialized: {
                // Fix imports
                auto singerSpec = static_cast<SingerSpec *>(spec);
                auto spec_d = static_cast<SingerSpec::Impl *>(singerSpec->_impl.get());
                for (auto &imp : spec_d->imports) {
                    ContributeIdentifier newIdentifier(
                        imp.inference.library().empty() ? spec->parent()->id()
                                                        : imp.inference.library(),
                        imp.inference.version().isEmpty() ? spec->parent()->version()
                                                          : imp.inference.version(),
                        imp.inference.id());
                    imp.inference = newIdentifier;
                }
                return ContributeRegistry::loadSpec(spec, state, error);
            }
            case ContributeSpec::Ready: {
                // Check inferences
                auto singerSpec = static_cast<SingerSpec *>(spec);
                auto inferenceReg =
                    impl.env->registry(ContributeSpec::Inference)->cast<InferenceRegistry>();
                for (const auto &imp : std::as_const(singerSpec->imports())) {
                    // Find inference
                    auto inferences = inferenceReg->findInferences(imp.inference);
                    if (inferences.empty()) {
                        *error = {
                            Error::FeatureNotSupported,
                            formatTextN(R"(required inference "%1" of singer "%2" not found)",
                                        imp.inference.toString(), singerSpec->id()),
                        };
                    }

                    // Validate
                    auto inference = inferences.front();
                    std::string errMsg;
                    if (!inference->validate(imp.options, &errMsg)) {
                        *error = {
                            Error::InvalidFormat,
                            formatTextN(
                                R"(inference "%1" of singer "%2" validate failed: %3)",
                                imp.inference.toString(), singerSpec->id(), errMsg),
                        };
                    }
                }
                return true;
            }
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

    SingerRegistry::SingerRegistry(Environment *env) : ContributeRegistry(*new Impl(env)) {
    }

}