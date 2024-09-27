#include "contributespec.h"
#include "contributespec_p.h"

namespace dsinfer {

    bool ContributeSpec::Impl::read(const std::filesystem::path &basePath, const JsonObject &obj,
                                    Error *error) {
        return false;
    }

    ContributeSpec::~ContributeSpec() = default;

    int ContributeSpec::type() const {
        __dsinfer_impl_t;
        return impl.type;
    }

    ContributeSpec::State ContributeSpec::state() const {
        __dsinfer_impl_t;
        return impl.state;
    }

    LibrarySpec *ContributeSpec::parent() const {
        __dsinfer_impl_t;
        return impl.parent;
    }

    std::string ContributeSpec::id() const {
        __dsinfer_impl_t;
        return impl.id;
    }

    ContributeSpec::ContributeSpec(Impl &impl) : _impl(&impl) {
    }

}