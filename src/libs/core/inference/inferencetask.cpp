#include "inferencetask.h"

namespace dsinfer {

    class InferenceTask::Impl {
    public:
        explicit Impl(Environment *env) : env(env) {
        }

        Environment *env;
    };

    InferenceTask::InferenceTask(Environment *env) : _impl(new Impl(env)) {
    }

    InferenceTask::~InferenceTask() = default;

    bool InferenceTask::start(const JsonValue &input, Error *error) {
        return false;
    }

    bool InferenceTask::stop() {
        return false;
    }

    InferenceTask::State InferenceTask::state() const {
        return InferenceTask::Idle;
    }

    JsonObject InferenceTask::result() const {
        return dsinfer::JsonObject();
    }

    Environment *InferenceTask::env() const {
        __dsinfer_impl_t;
        return impl.env;
    }

}