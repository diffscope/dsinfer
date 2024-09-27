#include "contributeregistry.h"
#include "contributeregistry_p.h"

#include "contributespec_p.h"

namespace dsinfer {

    void ContributeRegistry::Impl::setSpecState(ContributeSpec *spec, ContributeSpec::State state) {
        spec->_impl->state = state;
    }

    ContributeRegistry::~ContributeRegistry() = default;

    int ContributeRegistry::type() const {
        __dsinfer_impl_t;
        return impl.type;
    }

    Environment *ContributeRegistry::env() const {
        __dsinfer_impl_t;
        return impl.env;
    }

    ContributeRegistry::ContributeRegistry(Impl &impl) : _impl(&impl) {
    }


}