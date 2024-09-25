#ifndef PITCHINFERENCE_H
#define PITCHINFERENCE_H

#include <dsinfer/inference.h>

namespace dsinfer {

    class PitchInference : public Inference {
    public:
        explicit PitchInference(Environment *env);
        ~PitchInference();

    public:
        bool initialize(const dsinfer::JsonObject &args, std::string *error) override;

        bool start(const JsonValue &input, std::string *error) override;
        bool stop() override;

        State state() const override;
        JsonObject result() const override;

    protected:
        class Impl;
    };

}

#endif // PITCHINFERENCE_H
