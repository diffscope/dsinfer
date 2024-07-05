#ifndef DSINFERCORE_INFERENCE_H
#define DSINFERCORE_INFERENCE_H

#include <string>
#include <filesystem>

#include <dsinferCore/inferencemanifest.h>

namespace dsinfer {

    class DSINFER_CORE_EXPORT Inference {
    public:
        Inference();
        virtual ~Inference();

    public:
        virtual bool load(const InferenceInfo &info);

        virtual int start(const std::string &input);

    protected:
        DSINFER_DISABLE_COPY(Inference)
    };

}

#endif // DSINFERCORE_INFERENCE_H
