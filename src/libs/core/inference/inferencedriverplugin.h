#ifndef INFERENCEDRIVERPLUGIN_H
#define INFERENCEDRIVERPLUGIN_H

#include <dsinfer/plugin.h>
#include <dsinfer/inferencedriver.h>

namespace dsinfer {

    class DSINFER_EXPORT InferenceDriverPlugin : public Plugin {
    public:
        InferenceDriverPlugin();
        ~InferenceDriverPlugin();

        const char *iid() const override {
            return "com.diffsinger.InferenceDriver";
        }

    public:
        virtual InferenceDriver *create() const = 0;

    public:
        DSINFER_DISABLE_COPY(InferenceDriverPlugin)
    };

}

#endif // INFERENCEDRIVERPLUGIN_H
