#include "pitchinterpreter.h"

#include "pitchinference.h"

namespace dsinfer {

    PitchInterpreter::PitchInterpreter() = default;

    const char *PitchInterpreter::key() const {
        return "org.DiffSinger.PitchPrediction";
    }

    int PitchInterpreter::apiLevel() const {
        return 1;
    }

    bool PitchInterpreter::validate(const InferenceSpec *spec, std::string *message) const {
        return false;
    }

    bool PitchInterpreter::validate(const InferenceSpec *spec, const JsonObject &importOptions,
                                    std::string *message) const {
        return false;
    }

    Inference *PitchInterpreter::create(const InferenceSpec *spec, const JsonObject &options,
                                        Error *error) const {
        return nullptr;
    }

}

DSINFER_EXPORT_PLUGIN(dsinfer::PitchInterpreter)