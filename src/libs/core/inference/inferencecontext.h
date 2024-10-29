#ifndef INFERENCECONTEXT_H
#define INFERENCECONTEXT_H

#include <dsinfer/jsonvalue.h>

namespace dsinfer {

    class DSINFER_EXPORT InferenceContext {
    public:
        InferenceContext();
        virtual ~InferenceContext();

    public:
        virtual int64_t id() const = 0;

        virtual bool insertObject(const std::string &key, const JsonValue &value) = 0;
        virtual bool removeObject(const std::string &key) = 0;

        virtual bool containsObject(const std::string &key) const = 0;
        virtual JsonValue getObject(const std::string &key) const = 0;
        virtual void clearObjects() = 0;

        virtual bool executeCommand(const JsonValue &input, JsonValue *output) = 0;
    };

}


#endif // INFERENCECONTEXT_H
