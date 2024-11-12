#ifndef DSINFER_ONNXDRIVER_VALUEMAP_H
#define DSINFER_ONNXDRIVER_VALUEMAP_H

#include <map>
#include <memory>

#include <onnxruntime_cxx_api.h>

namespace dsinfer::onnxdriver {

    using ValueMap = std::map<std::string, Ort::Value>;
    using SharedValueMap = std::map<std::string, std::shared_ptr<Ort::Value>>;

    inline std::shared_ptr<Ort::Value> makeSharedValue(Ort::Value &&value) {
        return std::make_shared<Ort::Value>(std::move(value));
    }

    inline std::shared_ptr<Ort::Value> makeSharedValue(OrtValue *value) {
        return std::make_shared<Ort::Value>(value);
    }
}

#endif // DSINFER_ONNXDRIVER_VALUEMAP_H
