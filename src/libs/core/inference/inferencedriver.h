#ifndef INFERENCEDRIVER_H
#define INFERENCEDRIVER_H

#include <dsinfer/error.h>
#include <dsinfer/inferencesession.h>
#include <dsinfer/inferencetask.h>
#include <dsinfer/inferencecontext.h>

namespace dsinfer {

    class DSINFER_EXPORT InferenceDriver {
    public:
        InferenceDriver();
        virtual ~InferenceDriver();

    public:
        virtual bool initialize(const JsonValue &args, Error *error) = 0;

        virtual InferenceSession *createSession() = 0;
        virtual InferenceTask *createTask() = 0;
        virtual InferenceContext *createContext() = 0;

    public:
        DSINFER_DISABLE_COPY(InferenceDriver)
    };

}

#endif // INFERENCEDRIVER_H