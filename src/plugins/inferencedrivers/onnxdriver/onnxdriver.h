#ifndef ONNXDRIVER_H
#define ONNXDRIVER_H

#include <dsinfer/inferencedriver.h>

namespace dsinfer {

    class OnnxDriver : public InferenceDriver {
    public:
        OnnxDriver();
        ~OnnxDriver();

    public:
        bool initialize(const JsonValue &args, Error *error) override;

        InferenceSession *createSession() override ;
        InferenceTask *createTask() override ;
        InferenceContext *createContext() override;
    };

}

#endif // ONNXDRIVER_H
