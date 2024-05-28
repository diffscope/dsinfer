#include "acousticinterpreter.h"

namespace dsinfer {

    AcousticInterpreter::AcousticInterpreter() {
    }

    const char *AcousticInterpreter::key() const {
        return "svs.Acoustic";
    }

    int AcousticInterpreter::level() const {
        return 0;
    }

    bool AcousticInterpreter::load(const LibraryInfo &info, std::string *errorMessage) {
        return true;
    }

}

DSINFER_EXPORT_INTERPRETER(dsinfer::AcousticInterpreter)