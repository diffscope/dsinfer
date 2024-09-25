#include "libraryspec.h"
#include "libraryspec_p.h"

#include <map>

namespace dsinfer {

    bool LibraryDependency::operator==(const LibraryDependency &other) const {
        return id == other.id && version == other.version;
    }

    LibrarySpec::Impl::~Impl() {
        for (const auto &it : std::as_const(contributes)) {
            for (const auto &spec : it.second) {
                delete spec;
            }
        }
    }

    LibrarySpec::~LibrarySpec() = default;

    std::string LibrarySpec::id() const {
        __dsinfer_impl_t;
        return impl.id;
    }

    VersionNumber LibrarySpec::version() const {
        __dsinfer_impl_t;
        return impl.version;
    }

    VersionNumber LibrarySpec::compatVersion() const {
        __dsinfer_impl_t;
        return impl.compatVersion;
    }

    std::string LibrarySpec::description() const {
        __dsinfer_impl_t;
        return impl.description;
    }

    std::string LibrarySpec::vendor() const {
        __dsinfer_impl_t;
        return impl.vendor;
    }

    std::string LibrarySpec::copyright() const {
        __dsinfer_impl_t;
        return impl.copyright;
    }

    std::string LibrarySpec::url() const {
        __dsinfer_impl_t;
        return impl.url;
    }

    const std::vector<ContributeSpec *> &LibrarySpec::contributes(int type) const {
        __dsinfer_impl_t;
        auto it = impl.contributes.find(type);
        if (it == impl.contributes.end()) {
            static std::vector<ContributeSpec *> _empty;
            return _empty;
        }
        return it->second;
    }

    ContributeSpec *LibrarySpec::contribute(int type, const std::string &id) const {
        return nullptr;
    }

    const std::vector<LibraryDependency> &LibrarySpec::dependencies() const {
        __dsinfer_impl_t;
        return impl.dependencies;
    }

    bool LibrarySpec::hasError() const {
        return false;
    }
    
    std::string LibrarySpec::errorMessage() const {
        return {};
    }

    Environment *LibrarySpec::env() const {
        return nullptr;
    }

    LibrarySpec::LibrarySpec(Environment *env) : _impl(new Impl(env)) {
    }

}