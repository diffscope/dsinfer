#ifndef ONNXDRIVER_H
#define ONNXDRIVER_H

#include <dsinfer/inferencedriver.h>

namespace dsinfer {

    class OnnxDriver : public InferenceDriver {
    public:
        explicit OnnxDriver(const std::filesystem::path &runtimePath);
        ~OnnxDriver();

    public:
        bool initialize(const JsonValue &args, Error *error) override;

        InferenceSession *createSession() override;
        InferenceTask *createTask() override;
        InferenceContext *createContext() override;

    protected:
        class Impl;
        std::unique_ptr<Impl> _impl;
    };

}

#endif // ONNXDRIVER_H
