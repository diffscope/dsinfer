#include "inference.h"
#include "inference_p.h"

namespace dsinfer {

    Inference::Inference(Environment *env) : Inference(*new Impl(env)) {
    }

    Inference::~Inference() = default;

    Environment *Inference::env() const {
        __dsinfer_impl_t;
        return impl.env;
    }

    Inference::Inference(Impl &impl) : _impl(&impl) {
    }

}