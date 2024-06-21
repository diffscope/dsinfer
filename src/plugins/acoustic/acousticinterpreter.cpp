#include "acousticinterpreter.h"

#include "acousticinference.h"

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

    Inference *AcousticInterpreter::create() const {
        return new AcousticInference();
    }

}

DSINFER_EXPORT_INTERPRETER(dsinfer::AcousticInterpreter)