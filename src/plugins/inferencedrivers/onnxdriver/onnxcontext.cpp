#include "onnxcontext.h"
#include "onnxcontext_p.h"

#include <dsinfer/dsinferglobal.h>
#include <dsinfer/error.h>

#include "internal/onnxdriver_logger.h"
#include "internal/valueparser.h"

namespace dsinfer {

    static inline bool checkStringValue(
        const JsonObject &obj,
        const std::string &key,
        const std::string &value);

    static inline bool checkStringValues(
        const JsonObject &obj,
        const std::string &key,
        const std::initializer_list<std::string> &values);

    OnnxContext::OnnxContext()
        :_impl(std::make_unique<Impl>()) {
    }

    OnnxContext::~OnnxContext() {
    }

    int64_t OnnxContext::id() const {
        return reinterpret_cast<int64_t>(this);
    }

    bool OnnxContext::insertObject(const std::string &key, const JsonValue &value) {
        __dsinfer_impl_t;
        auto obj = value.toObject();
        if (!checkStringValue(obj, "type", "object")) {
            return false;
        }
        if (auto it_root = obj.find("content"); it_root != obj.end() && it_root->second.isObject()) {
            auto content = it_root->second.toObject();
            if (!checkStringValue(content, "class", "Ort::Value")) {
                return false;
            }
            if (auto it_content = content.find("data"); it_content != content.end()) {
                if (checkStringValue(content, "format", "bytes")) {
                    Error error;
                    auto ortVal = onnxdriver::deserializeTensor(it_content->second.toObject(), &error);
                    if (!error.ok()) {
                        onnxdriver_log().critical("OnnxContext - " + error.message());
                        return false;
                    }
                    // Save Ort::Value to value map
                    {
                        std::unique_lock<std::shared_mutex> lock(impl.mtx);
                        impl.valueMap[key] = onnxdriver::makeSharedValue(std::move(ortVal));
                    }
                    onnxdriver_log().info("OnnxContext - Inserted value \"%1\" to context", key);
                    return true;
                } else if (checkStringValue(content, "format", "array")) {
                    return false; // TODO: to be implemented
                }
            }
        }
        return false;
    }

    bool OnnxContext::removeObject(const std::string &key) {
        __dsinfer_impl_t;
        std::unique_lock<std::shared_mutex> lock(impl.mtx);
        if (auto it = impl.valueMap.find(key); it != impl.valueMap.end()) {
            impl.valueMap.erase(it);
            onnxdriver_log().info("OnnxContext - Removed value \"%1\" from context", key);
            return true;
        }
        return false;
    }

    bool OnnxContext::containsObject(const std::string &key) const {
        __dsinfer_impl_t;
        std::shared_lock<std::shared_mutex> lock(impl.mtx);
        return impl.valueMap.find(key) != impl.valueMap.end();
    }

    JsonValue OnnxContext::getObject(const std::string &key) const {
        __dsinfer_impl_t;
        std::shared_lock<std::shared_mutex> lock(impl.mtx);
        if (auto it = impl.valueMap.find(key); it != impl.valueMap.end()) {
            auto ortVal = it->second;
            Error error;
            auto jVal = onnxdriver::serializeTensor(*ortVal, &error);
            if (!error.ok()) {
                onnxdriver_log().critical("OnnxContext - " + error.message());
                return {};
            }
            return JsonObject{
                {"type", "object"},
                {"content", JsonObject{
                    {"class", "Ort::Value"},
                    {"format", "bytes"},
                    {"data", jVal}}
                }
            };
        }
        return {};
    }

    void OnnxContext::clearObjects() {
        __dsinfer_impl_t;
        std::unique_lock<std::shared_mutex> lock(impl.mtx);
        impl.valueMap.clear();
    }

    bool OnnxContext::executeCommand(const JsonValue &input, JsonValue *output) {
        // const auto &obj = input.toObject();
        // auto it = obj.find("command");
        // if (it == obj.end()) {
        //     *output = R"(no command)";
        //     return false;
        // }

        // auto cmd = it->second.toString();
        // if (cmd == "remove") {
        //     it = obj.find("key");
        //     if (it == obj.end()) {
        //         *output = R"(no key)";
        //         return false;
        //     }
        //     *output = "ok";
        //     return removeObject(it->second.toString());
        // }

        // No need to implement
        return false;
    }

    static inline bool checkStringValue(const JsonObject &obj, const std::string &key, const std::string &value) {
        if (auto it = obj.find(key); it != obj.end()) {
            if (!it->second.isString()) {
                return false;
            }
            return it->second.toString() == value;
        }
        return false;
    }

    static inline bool checkStringValues(const JsonObject &obj, const std::string &key, const std::initializer_list<std::string> &values) {
        if (auto it = obj.find(key); it != obj.end()) {
            if (!it->second.isString()) {
                return false;
            }
            const auto valString = it->second.toString();
            for (const auto &value : values) {
                if (valString == value) {
                    return true;
                }
            }
        }
        return false;
    }

}