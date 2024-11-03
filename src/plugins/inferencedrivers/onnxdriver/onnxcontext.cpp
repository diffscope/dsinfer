#include "onnxcontext.h"
#include "onnxcontext_p.h"

#include <dsinfer/dsinferglobal.h>
#include <dsinfer/error.h>

#include "internal/onnxdriver_logger.h"
#include "internal/idutil.h"
#include "internal/valueparser.h"

namespace dsinfer {

    static IdManager<OnnxContext> &idManager() {
        static IdManager<OnnxContext> manager;
        return manager;
    }

    OnnxContext::OnnxContext()
            :_impl(std::make_unique<Impl>()) {
        __dsinfer_impl_t;
        auto contextId = idManager().add(this);
        impl.contextId = contextId;
        onnxdriver_log().debug("OnnxContext [%1] - new context created", contextId);
    }

    OnnxContext::~OnnxContext() {
        __dsinfer_impl_t;
        idManager().remove(impl.contextId);
    }

    OnnxContext *OnnxContext::getContext(int64_t contextId) {
        return idManager().get(contextId);
    }

    int64_t OnnxContext::id() const {
        __dsinfer_impl_t;
        return impl.contextId;
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
            Error error;
            auto ortVal = onnxdriver::parseInputContent(content, &error);
            if (error.ok()) {
                // Save Ort::Value to value map
                {
                    std::unique_lock<std::shared_mutex> lock(impl.mtx);
                    impl.valueMap[key] = onnxdriver::makeSharedValue(std::move(ortVal));
                }
                onnxdriver_log().info("OnnxContext [%1] - Inserted value \"%2\" to context", impl.contextId, key);
                return true;
            } else {
                onnxdriver_log().critical("OnnxContext [%1] - Failed to insert value \"%2\" to context: %3", impl.contextId, key, error.what());
                return false;
            }
        }
        return false;
    }

    bool OnnxContext::removeObject(const std::string &key) {
        __dsinfer_impl_t;
        std::unique_lock<std::shared_mutex> lock(impl.mtx);
        if (auto it = impl.valueMap.find(key); it != impl.valueMap.end()) {
            impl.valueMap.erase(it);
            onnxdriver_log().info("OnnxContext [%1] - Removed value \"%2\" from context", impl.contextId, key);
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
                onnxdriver_log().critical("OnnxContext [%1] - %2", impl.contextId, error.message());
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



}