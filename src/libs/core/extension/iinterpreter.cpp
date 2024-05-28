#include "iinterpreter.h"

namespace dsinfer {

    IInterpreter::IInterpreter() = default;

    IInterpreter::~IInterpreter() = default;

    bool IInterpreter::load(const LibraryInfo &info, std::string *errorMessage) {
        return true;
    }

}