#include "inference.h"
#include "inference_p.h"

#include "inferencespec.h"

namespace dsinfer {

    Inference::Inference(const InferenceSpec *env) : Inference(*new Impl(env)) {
    }

    Inference::~Inference() = default;

    const InferenceSpec *Inference::spec() const {
        __stdc_impl_t;
        return impl.spec;
    }

    Environment *Inference::env() const {
        __stdc_impl_t;
        return impl.spec->env();
    }

    Inference::Inference(Impl &impl) : _impl(&impl) {
    }

}