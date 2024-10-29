#include <dsinfer/inferenceinterpreterplugin.h>

#include "acousticinterpreter.h"

namespace dsinfer {

    class AcousticInterpreterPlugin : public InferenceInterpreterPlugin {
    public:
        AcousticInterpreterPlugin() = default;

        const char *key() const {
            return "com.diffsinger.InferenceInterpreter.AcousticPrediction";
        }

        InferenceInterpreter *create() {
            return new AcousticInterpreter();
        }
    };

}

DSINFER_EXPORT_PLUGIN(dsinfer::AcousticInterpreterPlugin)