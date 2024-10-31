#include "onnxcontext.h"

namespace dsinfer {

    OnnxContext::OnnxContext() {
    }

    OnnxContext::~OnnxContext() {
    }

    int64_t OnnxContext::id() const {
        return reinterpret_cast<int64_t>(this);
    }

    bool OnnxContext::insertObject(const std::string &key, const JsonValue &value) {
        return false;
    }

    bool OnnxContext::removeObject(const std::string &key) {
        return false;
    }

    bool OnnxContext::containsObject(const std::string &key) const {
        return false;
    }

    JsonValue OnnxContext::getObject(const std::string &key) const {
        return {};
    }

    void OnnxContext::clearObjects() {
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

        // No need implement
        return false;
    }

}