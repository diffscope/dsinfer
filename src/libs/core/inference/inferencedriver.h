#ifndef INFERENCEDRIVER_H
#define INFERENCEDRIVER_H

#include <filesystem>

#include <dsinfer/error.h>
#include <dsinfer/jsonvalue.h>

namespace dsinfer {

    class DSINFER_EXPORT InferenceDriver {
    public:
        InferenceDriver();
        virtual ~InferenceDriver();

    public:
        virtual bool initialize(const JsonObject &args, Error *error) const = 0;

        virtual int64_t sessionCreate(const std::filesystem::path &path, const JsonObject &args,
                                      Error *error) const = 0;
        virtual bool sessionDestroy(int64_t handle, Error *error) const = 0;
        virtual bool sessionRunning(int64_t handle) const = 0;

        virtual int64_t taskCreate() const = 0;
        virtual void taskDestroy(int64_t handle) const = 0;
        virtual bool taskStart(int64_t handle, const JsonValue &input, Error *error) const = 0;
        virtual bool taskStop(int64_t handle, Error *error) const = 0;
        virtual int taskState(int64_t handle) const = 0;
        virtual bool taskResult(int64_t handle, JsonValue *result) const = 0;

    public:
        DSINFER_DISABLE_COPY(InferenceDriver)
    };

}

#endif // INFERENCEDRIVER_H
