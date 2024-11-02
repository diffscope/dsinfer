#ifndef ONNXCONTEXT_P_H
#define ONNXCONTEXT_P_H

#include <shared_mutex>

#include "onnxcontext.h"
#include "internal/valuemap.h"

namespace dsinfer {
    class OnnxContext::Impl {
    public:
        mutable std::shared_mutex mtx;
        onnxdriver::SharedValueMap valueMap;
    };
}

#endif // ONNXCONTEXT_P_H