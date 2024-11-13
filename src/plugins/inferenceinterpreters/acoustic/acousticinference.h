#ifndef ACOUSTICINFERENCE_H
#define ACOUSTICINFERENCE_H

#include <dsinfer/inference.h>

namespace dsinfer {

    class AcousticInference : public Inference {
    public:
        explicit AcousticInference(const InferenceSpec *spec);
        ~AcousticInference();

    public:
        bool initialize(const JsonValue &args, Error *error) override;

        bool start(const JsonValue &input, Error *error) override;
        bool startAsync(const JsonValue &input,
                        const std::function<void(const JsonValue &, const Error &)> &callback,
                        Error *error) override;
        bool stop() override;

        State state() const override;
        JsonValue result() const override;

    protected:
        class Impl;
    };

}

#endif // ACOUSTICINFERENCE_H
