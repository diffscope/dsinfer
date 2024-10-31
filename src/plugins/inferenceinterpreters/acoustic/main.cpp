#include <dsinfer/inferenceinterpreterplugin.h>

#include "acousticinterpreter.h"

namespace dsinfer {

    class AcousticInterpreterPlugin : public InferenceInterpreterPlugin {
    public:
        AcousticInterpreterPlugin() = default;

        const char *key() const {
            return "ai.svs.AcousticInference";
        }

        InferenceInterpreter *create() {
            return new AcousticInterpreter();
        }
    };

}

DSINFER_EXPORT_PLUGIN(dsinfer::AcousticInterpreterPlugin)