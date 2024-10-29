#ifndef ONNXDRIVER_H
#define ONNXDRIVER_H

#include <dsinfer/inferencedriver.h>

namespace dsinfer {

    class OnnxDriver : public InferenceDriver {
    public:
        OnnxDriver();
        ~OnnxDriver();

    public:
        bool initialize(const JsonValue &args, Error *error) override {
            return {};
        }

        InferenceSession *createSession() override {
            return {};
        }

        InferenceTask *createTask() override {
            return {};
        }

        InferenceContext *createContext() override {
            return {};
        }
    };

}

#endif // ONNXDRIVER_H
