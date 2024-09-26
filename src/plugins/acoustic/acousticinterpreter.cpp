#include "acousticinterpreter.h"

#include "acousticinference.h"

#include <dsinfer/format.h>

namespace dsinfer {

    AcousticInterpreter::AcousticInterpreter() = default;

    const char *AcousticInterpreter::key() const {
        return "org.DiffSinger.AcousticPrediction";
    }

    int AcousticInterpreter::apiLevel() const {
        return 1;
    }

    bool AcousticInterpreter::validate(const InferenceSpec *spec, std::string *message) const {
        return false;
    }

    bool AcousticInterpreter::validate(const InferenceSpec *spec, const JsonObject &importOptions,
                                       std::string *message) const {
        return false;
    }

    Inference *AcousticInterpreter::create(const InferenceSpec *spec, const JsonObject &options,
                                           Error *error) const {
        switch (spec->apiLevel()) {
            case 1:
                return new AcousticInference(spec->env());
            case 2:
                return new AcousticInference(spec->env());
            default:
                break;
        }
        *error = {
            Error::FeatureNotSupported,
            formatTextN("Acoustic api level %1 not supported", spec->apiLevel()),
        };
        return nullptr;
    }

}

DSINFER_EXPORT_PLUGIN(dsinfer::AcousticInterpreter)