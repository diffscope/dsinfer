#ifndef INFERENCEDRIVERPLUGIN_H
#define INFERENCEDRIVERPLUGIN_H

#include <stdcorelib/plugin.h>

#include <dsinfer/inferencedriver.h>

namespace dsinfer {

    class DSINFER_EXPORT InferenceDriverPlugin : public stdc::Plugin {
    public:
        InferenceDriverPlugin();
        ~InferenceDriverPlugin();

        const char *iid() const override {
            return "com.diffsinger.InferenceDriver";
        }

    public:
        virtual InferenceDriver *create() = 0;

    public:
        STDCORELIB_DISABLE_COPY(InferenceDriverPlugin)
    };

}

#endif // INFERENCEDRIVERPLUGIN_H
