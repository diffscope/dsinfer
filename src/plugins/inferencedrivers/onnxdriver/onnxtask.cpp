#include "onnxtask.h"

namespace dsinfer {

    OnnxTask::OnnxTask() {
    }

    OnnxTask::~OnnxTask() {
    }

    bool OnnxTask::initialize(const JsonValue &args, Error *error) {
        return false;
    }

    bool OnnxTask::start(const JsonValue &input, Error *error) {
        return false;
    }

    bool OnnxTask::stop(Error *error) {
        return false;
    }

    int64_t OnnxTask::id() const {
        return reinterpret_cast<int64_t>(this);
    }

    InferenceTask::State OnnxTask::state() const {
        return InferenceTask::Terminated;
    }

    JsonValue OnnxTask::result() const {
        return {};
    }

}