#include "onnxdriverplugin.h"

#include "onnxdriver.h"

namespace dsinfer {

    OnnxDriverPlugin::OnnxDriverPlugin() {
    }

    const char *OnnxDriverPlugin::key() const {
        return "org.DiffSinger.InferenceDriver.OnnxDriver";
    }

    InferenceDriver *OnnxDriverPlugin::create() const {
        return new OnnxDriver();
    }

}

DSINFER_EXPORT_PLUGIN(dsinfer::OnnxDriverPlugin)