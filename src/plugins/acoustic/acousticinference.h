#ifndef ACOUSTICINFERENCE_H
#define ACOUSTICINFERENCE_H

#include <string>

#include <dsinferCore/inference.h>

namespace dsinfer {

    class AcousticInference : public Inference {
    public:
        bool exec(const std::string &input, std::string &output) const override;
    };

}

#endif // ACOUSTICINFERENCE_H
