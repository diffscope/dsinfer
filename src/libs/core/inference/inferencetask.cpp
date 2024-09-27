#include "inferencetask.h"

#include "inferenceregistry.h"
#include "inferencespec.h"

namespace dsinfer {

    class InferenceTask::Impl {
    public:
        explicit Impl(Environment *env) : env(env) {
            reg = env->registry(ContributeSpec::Inference)->cast<InferenceRegistry>();
            id = reg->driver()->taskCreate();
        }

        ~Impl() {
            reg->driver()->taskDestroy(id);
        }

        Environment *env;
        InferenceRegistry *reg;

        int64_t id;
    };

    InferenceTask::InferenceTask(Environment *env) : _impl(new Impl(env)) {
    }

    InferenceTask::~InferenceTask() = default;

    bool InferenceTask::start(const JsonValue &input, Error *error) {
        __dsinfer_impl_t;
        return impl.reg->driver()->taskStart(impl.id, input, error);
    }

    bool InferenceTask::stop(Error *error) {
        __dsinfer_impl_t;
        return impl.reg->driver()->taskStop(impl.id, error);
    }

    int64_t InferenceTask::id() const {
        __dsinfer_impl_t;
        return impl.id;
    }

    InferenceTask::State InferenceTask::state() const {
        __dsinfer_impl_t;
        return static_cast<State>(impl.reg->driver()->taskState(impl.id));
    }

    JsonValue InferenceTask::result() const {
        __dsinfer_impl_t;
        JsonValue result;
        if (!impl.reg->driver()->taskResult(impl.id, &result)) {
            return {};
        }
        return result;
    }

    Environment *InferenceTask::env() const {
        __dsinfer_impl_t;
        return impl.env;
    }

}