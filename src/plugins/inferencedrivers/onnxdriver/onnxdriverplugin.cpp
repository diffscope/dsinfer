#include "onnxdriverplugin.h"

#include "onnxdriver.h"

namespace dsinfer {

    OnnxDriverPlugin::OnnxDriverPlugin() {
    }

    const char *OnnxDriverPlugin::key() const {
        return "com.diffsinger.InferenceDriver.OnnxDriver";
    }

    InferenceDriver *OnnxDriverPlugin::create() {
        return new OnnxDriver();
    }

}

DSINFER_EXPORT_PLUGIN(dsinfer::OnnxDriverPlugin)