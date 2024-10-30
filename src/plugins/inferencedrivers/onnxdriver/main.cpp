#include <dsinfer/inferencedriverplugin.h>

#include "onnxdriver.h"

namespace dsinfer {

    class OnnxDriverPlugin : public InferenceDriverPlugin {
    public:
        OnnxDriverPlugin() = default;

    public:
        const char *key() const override {
            return "com.diffsinger.InferenceDriver.OnnxDriver";
        }

    public:
        InferenceDriver *create() override {
            return new OnnxDriver();
        }
    };

}

DSINFER_EXPORT_PLUGIN(dsinfer::OnnxDriverPlugin)