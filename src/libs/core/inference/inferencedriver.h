#ifndef INFERENCEDRIVER_H
#define INFERENCEDRIVER_H

#include <dsinfer/jsonvalue.h>

namespace dsinfer {

    class DSINFER_EXPORT InferenceDriver {
    public:
        InferenceDriver();
        virtual ~InferenceDriver();

        enum SessionAttribute {
            SA_ErrorMessage = 0x1,
            SA_Load,
            SA_TaskCount,
        };

        enum TaskAttribute {
            TA_ErrorMessage = 0x1,
            TA_Input,
            TA_Output,
            TA_Running,
        };

    public:
        virtual bool initialize(const JsonObject &args, const std::string *error) = 0;

        virtual int64_t sessionCreate(const JsonObject &args) const = 0;
        virtual int64_t sessionDestroy(int64_t id) const = 0;
        virtual int64_t sessionAttributeGet(int64_t id, int attr, JsonValue *out) const = 0;
        virtual int64_t sessionAttributeSet(int64_t id, int attr, const JsonValue &in) const = 0;

        virtual int64_t taskCreate(const JsonObject &args) const = 0;
        virtual int64_t taskDestroy(int64_t id) const = 0;
        virtual int64_t taskStart(int64_t id) const = 0;
        virtual int64_t taskStop() const = 0;
        virtual int64_t taskAttributeGet(int64_t id, int attr, JsonValue *out) const = 0;
        virtual int64_t taskAttributeSet(int64_t id, int attr, const JsonValue &in) const = 0;

    public:
        DSINFER_DISABLE_COPY(InferenceDriver)
    };

}

#endif // INFERENCEDRIVER_H
