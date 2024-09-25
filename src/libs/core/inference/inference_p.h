#ifndef INFERENCE_P_H
#define INFERENCE_P_H

#include <dsinfer/inference.h>

namespace dsinfer {

    class DSINFER_EXPORT Inference::Impl {
    public:
        explicit Impl(Environment *env) : env(env) {
        }
        virtual ~Impl() = default;

        Environment *env;
    };

}

#endif // INFERENCE_P_H
