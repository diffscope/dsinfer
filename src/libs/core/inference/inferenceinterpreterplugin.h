#ifndef INFERENCEINTERPRETERPLUGIN_H
#define INFERENCEINTERPRETERPLUGIN_H

#include <dsinfer/plugin.h>
#include <dsinfer/inferenceinterpreter.h>

namespace dsinfer {

    class DSINFER_EXPORT InferenceInterpreterPlugin : public Plugin {
    public:
        InferenceInterpreterPlugin();
        ~InferenceInterpreterPlugin();

        const char *iid() const override {
            return "com.diffsinger.InferenceInterpreter";
        }

    public:
        virtual InferenceInterpreter *create() = 0;

    public:
        DSINFER_DISABLE_COPY(InferenceInterpreterPlugin)
    };

}

#endif // INFERENCEINTERPRETERPLUGIN_H
