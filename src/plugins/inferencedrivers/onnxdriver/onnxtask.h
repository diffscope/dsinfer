#ifndef ONNXTASK_H
#define ONNXTASK_H

#include <dsinfer/inferencetask.h>

namespace dsinfer {

    class OnnxTask : public InferenceTask {
    public:
        OnnxTask();
        ~OnnxTask();

    public:
        bool initialize(const JsonValue &args, Error *error) override;

        bool start(const JsonValue &input, Error *error) override;
        bool stop(Error *error) override;

        int64_t id() const override;
        State state() const override;
        JsonValue result() const override;
    };

}

#endif // ONNXTASK_H
