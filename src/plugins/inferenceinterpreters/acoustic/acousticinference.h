#ifndef ACOUSTICINFERENCE_H
#define ACOUSTICINFERENCE_H

#include <string>

#include <dsinfer/inference.h>

namespace dsinfer {

    class AcousticInference : public Inference {
    public:
        explicit AcousticInference(const InferenceSpec *spec);
        ~AcousticInference();

    public:
        bool initialize(const JsonObject &args, Error *error) override;

        bool start(const JsonValue &input, Error *error) override;
        bool stop() override;

        State state() const override;
        JsonValue result() const override;

    protected:
        class Impl;
    };

}

#endif // ACOUSTICINFERENCE_H
