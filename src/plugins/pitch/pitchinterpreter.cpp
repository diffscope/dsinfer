#include "pitchinterpreter.h"

#include "pitchinference.h"

namespace dsinfer {

    PitchInterpreter::PitchInterpreter() {
    }

    const char *PitchInterpreter::key() const {
        return "svs.Variance.Pitch";
    }

    int PitchInterpreter::level() const {
        return 0;
    }

    bool PitchInterpreter::load(const LibrarySpec &info, std::string *errorMessage) {
        return true;
    }

    Inference *PitchInterpreter::create() const {
        return new PitchInference();
    }

}

DSINFER_EXPORT_INTERPRETER(dsinfer::PitchInterpreter)