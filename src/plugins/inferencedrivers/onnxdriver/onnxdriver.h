#ifndef ONNXDRIVER_H
#define ONNXDRIVER_H

#include <dsinfer/inferencedriver.h>

namespace dsinfer {

    class OnnxDriver : public InferenceDriver {
    public:
        OnnxDriver();
        ~OnnxDriver();

    public:
        /**
         * Initialize onnx inference driver.
         * @param args Initialization parameters:
         * {
         *   "lib": "/path/to/onnxruntime",
         *   "ep": "dml",
         *   "deviceIndex": 0
         * }
         * @param error Output error information (optional, can be nullptr)
         * @return Whether initialization is successful.
         */
        bool initialize(const JsonValue &args, Error *error) override;

        /**
         * Create onnx inference session.
         * @param path Path to .onnx file
         * @param args Session creation parameters:
         * {
         *   "preferCpu": false
         * }
         * @param error Output error information (optional, can be nullptr)
         * @return Session handle. If errors occur, return 0.
         */
        int64_t sessionCreate(const std::filesystem::path &path, const JsonValue &args,
                              Error *error) override;
        bool sessionDestroy(int64_t handle, Error *error) override;
        bool sessionRunning(int64_t handle) override;

        int64_t taskCreate() override;
        void taskDestroy(int64_t handle) override;

        /**
         * Start onnx inference task.
         * @param handle Task ID
         * @param input See taskStart.json5
         * @param error Output error information (optional, can be nullptr)
         * @return Whether the task started successfully
         */
        bool taskStart(int64_t handle, const JsonValue &input, Error *error) override;
        bool taskStop(int64_t handle, Error *error) override;
        int taskState(int64_t handle) override;
        bool taskResult(int64_t handle, JsonValue *result) override;

    protected:
        class Impl;
        std::unique_ptr<Impl> _impl;
    };


}

#endif // ONNXDRIVER_H
