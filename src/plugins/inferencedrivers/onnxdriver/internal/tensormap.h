#ifndef ONNXDRIVER_TENSORMAP_H
#define ONNXDRIVER_TENSORMAP_H

#include <map>
#include <string>
#include <memory>

#include <onnxruntime_cxx_api.h>

namespace dsinfer {
    namespace onnxdriver {
        //using ValueMap = std::map<std::string, Ort::Value>;
        //using ValueRefMap = std::map<std::string, std::reference_wrapper<Ort::Value>>;
        using SharedValue = std::shared_ptr<Ort::Value>;
        using ValueMap = std::map<std::string, SharedValue>;

        inline SharedValue makeSharedValue(Ort::Value &&value) {
            return std::make_shared<Ort::Value>(std::move(value));
        }
    }
}
#endif // ONNXDRIVER_TENSORMAP_H