#ifndef INFERENCE_H
#define INFERENCE_H

#include <string>
#include <filesystem>

#include <dsinferCore/inferenceinfo.h>

namespace dsinfer {

    class DSINFER_CORE_EXPORT Inference {
    public:
        Inference();
        virtual ~Inference();

    public:
    virtual bool load()
        virtual int start(const std::string &input);

    protected:
        DSINFER_DISABLE_COPY(Inference)
    };

}

#endif // INFERENCE_H
