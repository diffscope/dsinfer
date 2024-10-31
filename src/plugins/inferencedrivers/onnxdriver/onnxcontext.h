#ifndef ONNXCONTEXT_H
#define ONNXCONTEXT_H

#include <dsinfer/inferencecontext.h>

namespace dsinfer {

    class OnnxContext : public InferenceContext {
    public:
        OnnxContext();
        ~OnnxContext();

    public:
        int64_t id() const override;

        bool insertObject(const std::string &key, const JsonValue &value) override;
        bool removeObject(const std::string &key) override;

        bool containsObject(const std::string &key) const override;
        JsonValue getObject(const std::string &key) const override;
        void clearObjects() override;

        bool executeCommand(const JsonValue &input, JsonValue *output) override;
    };

}

#endif // ONNXCONTEXT_H
