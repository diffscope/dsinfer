#include <dsinfer/inferencedriverplugin.h>

#include "onnxdriver.h"

namespace dsinfer {

    class OnnxDriverPlugin : public InferenceDriverPlugin {
    public:
        OnnxDriverPlugin() = default;

    public:
        const char *key() const override {
            return "onnx";
        }

    public:
        InferenceDriver *create() override {
            return new OnnxDriver(path().parent_path() / _TSTR("runtimes"));
        }
    };

}

STDCORELIB_EXPORT_PLUGIN(dsinfer::OnnxDriverPlugin)