#include "iinterpreter.h"

namespace dsinfer {

    IInterpreter::IInterpreter() = default;

    IInterpreter::~IInterpreter() = default;

    bool IInterpreter::load() {
        return true;
    }

}