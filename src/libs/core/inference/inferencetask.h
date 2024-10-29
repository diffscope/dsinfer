#ifndef INFERENCETASK_H
#define INFERENCETASK_H

#include <dsinfer/error.h>
#include <dsinfer/jsonvalue.h>

namespace dsinfer {

    class DSINFER_EXPORT InferenceTask {
    public:
        InferenceTask();
        virtual ~InferenceTask();

        enum State {
            Idle,
            Running,
            Failed,
            Terminated,
        };

    public:
        virtual bool initialize(const JsonValue &args, Error *error) = 0;

        virtual bool start(const JsonValue &input, Error *error) = 0;
        virtual bool stop(Error *error) = 0;

        virtual int64_t id() const = 0;
        virtual State state() const = 0;
        virtual JsonValue result() const = 0;
    };

}

#endif // INFERENCETASK_H
