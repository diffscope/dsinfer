#ifndef INFERENCEINTERPRETER_H
#define INFERENCEINTERPRETER_H

#include <dsinfer/plugin.h>
#include <dsinfer/inferencespec.h>
#include <dsinfer/inference.h>

namespace dsinfer {

    class DSINFER_EXPORT InferenceInterpreter : public Plugin {
    public:
        InferenceInterpreter();
        virtual ~InferenceInterpreter();

        const char *iid() const override {
            return "org.OpenVPI.InferenceInterpreter";
        }

    public:
        virtual int apiLevel() const = 0;

        virtual bool validate(const InferenceSpec *spec, std::string *message) const;
        virtual bool validate(const InferenceSpec *spec, const JsonValue &importOptions,
                              std::string *message) const;

        virtual Inference *create(const InferenceSpec *spec, const JsonValue &options,
                                  Error *error) const = 0;

    public:
        DSINFER_DISABLE_COPY(InferenceInterpreter)
    };

}

#endif // INFERENCEINTERPRETER_H
