#ifndef ONNXCONTEXT_P_H
#define ONNXCONTEXT_P_H

#include <shared_mutex>

#include "onnxcontext.h"
#include "internal/valuemap.h"

namespace dsinfer {
    class OnnxContext::Impl {
    public:
        mutable std::shared_mutex mtx;
        int64_t contextId = 0;
        onnxdriver::SharedValueMap valueMap;

        std::shared_ptr<Ort::Value> getOrtValue(const std::string &key) {
            std::shared_lock<std::shared_mutex> lock(mtx);
            if (auto it = valueMap.find(key); it != valueMap.end()) {
                return it->second;
            }
            return nullptr;
        }
    };
}

#endif // ONNXCONTEXT_P_H