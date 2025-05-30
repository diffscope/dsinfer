#ifndef ACOUSTICINTERPRETER_H
#define ACOUSTICINTERPRETER_H

#include <dsinfer/inferenceinterpreter.h>

namespace dsinfer {

    class AcousticInterpreter : public InferenceInterpreter {
    public:
        AcousticInterpreter();

    public:
        int apiLevel() const override;

        bool validate(const InferenceSpec *spec, std::string *message) const override;
        bool validate(const InferenceSpec *spec, const JsonValue &importOptions,
                      std::string *message) const override;

        virtual Inference *create(const InferenceSpec *spec, const JsonValue &options,
                                  Error *error) override;
    };

}

#endif // ACOUSTICINTERPRETER_H
