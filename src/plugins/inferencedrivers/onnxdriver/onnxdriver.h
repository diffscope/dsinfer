#ifndef ONNXDRIVER_H
#define ONNXDRIVER_H

#include <dsinfer/inferencedriver.h>

namespace dsinfer {

    class OnnxDriver : public InferenceDriver {
    public:
        OnnxDriver();
        ~OnnxDriver();

    public:
        bool initialize(const JsonObject &args, Error *error) const override;

        int64_t sessionCreate(const std::filesystem::path &path, const JsonObject &args,
                              Error *error) const override;
        bool sessionDestroy(int64_t handle, Error *error) const override;
        bool sessionRunning(int64_t handle) const override;

        int64_t taskCreate() const override;
        void taskDestroy(int64_t handle) const override;
        bool taskStart(int64_t handle, const JsonValue &input, Error *error) const override;
        bool taskStop(int64_t handle, Error *error) const override;
        int taskState(int64_t handle) const override;
        bool taskResult(int64_t handle, JsonValue *result) const override;
    };

}

#endif // ONNXDRIVER_H
