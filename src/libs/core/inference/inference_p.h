#ifndef INFERENCE_P_H
#define INFERENCE_P_H

#include <dsinfer/inference.h>

namespace dsinfer {

    class DSINFER_EXPORT Inference::Impl {
    public:
        explicit Impl(const InferenceSpec *spec) : spec(spec) {
        }
        virtual ~Impl() = default;

        const InferenceSpec *spec;
    };

}

#endif // INFERENCE_P_H
