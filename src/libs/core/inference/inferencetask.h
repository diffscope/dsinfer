#ifndef INFERENCETASK_H
#define INFERENCETASK_H

#include <dsinfer/jsonvalue.h>
#include <dsinfer/environment.h>

namespace dsinfer {

    class DSINFER_EXPORT InferenceTask {
    public:
        explicit InferenceTask(Environment *env);
        ~InferenceTask();

        enum State {
            Idle,
            Running,
            Failed,
            Terminated,
        };

    public:
        bool start(const JsonValue &input, Error *error);
        bool stop(Error *error);

        int64_t id() const;
        State state() const;
        JsonValue result() const;

    public:
        Environment *env() const;

    protected:
        class Impl;
        std::unique_ptr<Impl> _impl;
    };

}

#endif // INFERENCETASK_H
