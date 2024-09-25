#include "contributeregistry.h"
#include "contributeregistry_p.h"

namespace dsinfer {

    ContributeRegistry::~ContributeRegistry() = default;

    Environment *ContributeRegistry::env() const {
        __dsinfer_impl_t;
        return impl.env;
    }

    ContributeRegistry::ContributeRegistry(Impl &impl) : _impl(&impl) {
    }

}