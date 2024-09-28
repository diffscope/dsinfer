#include "inferenceinterpreter.h"

namespace dsinfer {

    InferenceInterpreter::InferenceInterpreter() = default;

    InferenceInterpreter::~InferenceInterpreter() = default;

    bool InferenceInterpreter::validate(const InferenceSpec *spec, std::string *message) const {
        return false;
    }

    bool InferenceInterpreter::validate(const InferenceSpec *spec, const JsonValue &importOptions,
                                        std::string *message) const {
        return false;
    }

}