#ifndef PITCHINFERENCE_H
#define PITCHINFERENCE_H

#include <dsinfer/inference.h>

namespace dsinfer {

    class PitchInference : public Inference {
    public:
        explicit PitchInference(Environment *env);
        ~PitchInference();

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

#endif // PITCHINFERENCE_H
