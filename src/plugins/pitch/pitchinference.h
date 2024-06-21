#ifndef PITCHINFERENCE_H
#define PITCHINFERENCE_H

#include <string>

#include <dsinferCore/inference.h>

namespace dsinfer {

    class PitchInference : public Inference {
    public:
        bool exec(const std::string &input, std::string &output) const override;
    };

}

#endif // PITCHINFERENCE_H
