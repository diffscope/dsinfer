#ifndef ONNXCONTEXT_H
#define ONNXCONTEXT_H

#include <memory>

#include <dsinfer/inferencecontext.h>

namespace dsinfer {

    class OnnxContext : public InferenceContext {
    public:
        OnnxContext();
        ~OnnxContext();

        static OnnxContext *getContext(int64_t contextId);

    public:
        int64_t id() const override;

        bool insertObject(const std::string &key, const JsonValue &value) override;
        bool removeObject(const std::string &key) override;

        bool containsObject(const std::string &key) const override;
        JsonValue getObject(const std::string &key) const override;
        void clearObjects() override;

        bool executeCommand(const JsonValue &input, JsonValue *output) override;

    protected:
        class Impl;
        std::unique_ptr<Impl> _impl;

        friend class OnnxTask;
    };

}

#endif // ONNXCONTEXT_H
