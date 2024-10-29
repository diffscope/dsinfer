#include "acousticinterpreter.h"

#include "acousticinference.h"

#include <dsinfer/format.h>

namespace dsinfer {

    AcousticInterpreter::AcousticInterpreter() = default;

    int AcousticInterpreter::apiLevel() const {
        return 1;
    }

    bool AcousticInterpreter::validate(const InferenceSpec *spec, std::string *message) const {
        return false;
    }

    bool AcousticInterpreter::validate(const InferenceSpec *spec, const JsonValue &importOptions,
                                       std::string *message) const {
        return false;
    }

    Inference *AcousticInterpreter::create(const InferenceSpec *spec, const JsonValue &options,
                                           Error *error) {
        switch (spec->apiLevel()) {
            case 1:
                return new AcousticInference(spec);
            case 2:
                return new AcousticInference(spec);
            default:
                break;
        }
        *error = {
            Error::FeatureNotSupported,
            formatTextN("acoustic api level %1 not supported", spec->apiLevel()),
        };
        return nullptr;
    }

}