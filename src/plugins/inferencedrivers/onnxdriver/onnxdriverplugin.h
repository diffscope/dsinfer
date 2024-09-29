#ifndef ONNXDRIVERPLUGIN_H
#define ONNXDRIVERPLUGIN_H

#include <dsinfer/inferencedriverplugin.h>

namespace dsinfer {

    class OnnxDriverPlugin : public InferenceDriverPlugin {
    public:
        OnnxDriverPlugin();

    public:
        const char *key() const override;

    public:
        InferenceDriver * create() const override;
    };

}

#endif // ONNXDRIVERPLUGIN_H
