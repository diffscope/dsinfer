#ifndef PITCHINTERPRETER_H
#define PITCHINTERPRETER_H

#include <dsinfer/inferenceinterpreter.h>

namespace dsinfer {

    class PitchInterpreter : public InferenceInterpreter {
    public:
        PitchInterpreter();

    public:
        const char *key() const override;

    public:
        int apiLevel() const override;

        bool validate(const InferenceSpec *spec, std::string *message) const override;
        bool validate(const InferenceSpec *spec, const JsonObject &importOptions,
                      std::string *message) const override;

        virtual Inference *create(const InferenceSpec *spec, const JsonObject &options,
                                  std::string *error) const override;
    };

}

#endif // PITCHINTERPRETER_H
