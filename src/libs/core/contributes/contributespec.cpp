#include "contributespec.h"
#include "contributespec_p.h"

namespace dsinfer {

    ContributeSpec::~ContributeSpec() = default;

    int ContributeSpec::type() const {
        __dsinfer_impl_t;
        return impl.type;
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