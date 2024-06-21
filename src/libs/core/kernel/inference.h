#ifndef INFERENCE_H
#define INFERENCE_H

#include <string>
#include <filesystem>

#include <dsinferCore/dsinfercoreglobal.h>

namespace dsinfer {

    class DSINFER_CORE_EXPORT Inference {
    public:
        Inference();
        virtual ~Inference();

    public:
        virtual bool exec(const std::string &input, std::string &output) const = 0;

    protected:
        DSINFER_DISABLE_COPY(Inference)
    };

}

#endif // INFERENCE_H
