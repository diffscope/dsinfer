#ifndef INFERENCEINTERPRETERPLUGIN_H
#define INFERENCEINTERPRETERPLUGIN_H

#include <stdcorelib/plugin.h>

#include <dsinfer/inferenceinterpreter.h>

namespace dsinfer {

    class DSINFER_EXPORT InferenceInterpreterPlugin : public stdc::Plugin {
    public:
        InferenceInterpreterPlugin();
        ~InferenceInterpreterPlugin();

        const char *iid() const override {
            return "com.diffsinger.InferenceInterpreter";
        }

    public:
        virtual InferenceInterpreter *create() = 0;

    public:
        STDCORELIB_DISABLE_COPY(InferenceInterpreterPlugin)
    };

}

#endif // INFERENCEINTERPRETERPLUGIN_H
