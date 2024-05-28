#include "pitchinterpreter.h"

namespace dsinfer {

    PitchInterpreter::PitchInterpreter() {
    }

    const char *PitchInterpreter::key() const {
        return "svs.Variance.Pitch";
    }

    int PitchInterpreter::level() const {
        return 0;
    }

    bool PitchInterpreter::load(const LibraryInfo &info, std::string *errorMessage) {
        return true;
    }

}

DSINFER_EXPORT_INTERPRETER(dsinfer::PitchInterpreter)