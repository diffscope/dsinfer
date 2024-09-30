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

        int64_t sessionCreate(const std::filesystem::path &path, const JsonValue &args,
                              Error *error) override;
        bool sessionDestroy(int64_t handle, Error *error) override;
        bool sessionRunning(int64_t handle) override;

        int64_t taskCreate() override;
        void taskDestroy(int64_t handle) override;
        bool taskStart(int64_t handle, const JsonValue &input, Error *error) override;
        bool taskStop(int64_t handle, Error *error) override;
        int taskState(int64_t handle) override;
        bool taskResult(int64_t handle, JsonValue *result) override;
    };

}

#endif // ONNXDRIVER_H
