#include "contributeregistry.h"
#include "contributeregistry_p.h"

#include <mutex>
#include <utility>

namespace dsinfer {

    std::vector<ContributeSpec *> ContributeRegistry::Impl::findContributes(
        const dsinfer::ContributeIdentifier &identifier) const {
        std::shared_lock<std::shared_mutex> lock(env_mtx());
        if (identifier.library().empty() || identifier.version().isEmpty()) {
            return {};
        }
        auto it = indexes.find(identifier.library());
        if (it == indexes.end()) {
            return {};
        }
        const auto &versionMap = it->second;

        auto it2 = versionMap.find(identifier.version());
        if (it2 == versionMap.end()) {
            return {};
        }
        const auto &inferenceMap = it2->second;

        if (!identifier.id().empty()) {
            auto it3 = inferenceMap.find(identifier.id());
            if (it3 == inferenceMap.end()) {
                return {};
            }
            return {*it3->second};
        }

        std::vector<ContributeSpec *> res;
        res.reserve(inferenceMap.size());
        for (const auto &pair : inferenceMap) {
            res.push_back(*pair.second);
        }
        return res;
    }

    ContributeRegistry::~ContributeRegistry() = default;

    int ContributeRegistry::type() const {
        __stdc_impl_t;
        return impl.type;
    }

    Environment *ContributeRegistry::env() const {
        __stdc_impl_t;
        return impl.env;
    }

    bool ContributeRegistry::loadSpec(ContributeSpec *spec, ContributeSpec::State state,
                                      Error *error) {
        __stdc_impl_t;
        switch (state) {
            case ContributeSpec::Initialized: {
                std::unique_lock<std::shared_mutex> lock(impl.env_mtx());
                auto lib = spec->parent();
                auto it = impl.contributes.insert(impl.contributes.end(), spec);
                impl.indexes[lib->id()][lib->version()][spec->id()] = it;
                return true;
            }
            case ContributeSpec::Ready:
            case ContributeSpec::Finished: {
                return true;
            }
            case ContributeSpec::Deleted: {
                std::unique_lock<std::shared_mutex> lock(impl.env_mtx());
                auto lib = spec->parent();
                auto it = impl.indexes.find(lib->id());
                if (it == impl.indexes.end()) {
                    return true;
                }
                auto &versionMap = it->second;
                auto it2 = versionMap.find(lib->version());
                if (it2 == versionMap.end()) {
                    return true;
                }
                auto &inferenceMap = it2->second;
                auto it3 = inferenceMap.find(spec->id());
                if (it3 == inferenceMap.end()) {
                    return true;
                }
                impl.contributes.erase(it3->second);
                inferenceMap.erase(it3);
                if (inferenceMap.empty()) {
                    versionMap.erase(it2);
                    if (versionMap.empty()) {
                        impl.indexes.erase(it);
                    }
                }
                return true;
            }
            default:
                break;
        }
        return false;
    }

    ContributeRegistry::ContributeRegistry(Impl &impl) : _impl(&impl) {
    }

}