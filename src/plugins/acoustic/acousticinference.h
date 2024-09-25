#ifndef ACOUSTICINFERENCE_H
#define ACOUSTICINFERENCE_H

#include <string>

#include <dsinfer/inference.h>

namespace dsinfer {

    class AcousticInference : public Inference {
    public:
        explicit AcousticInference(Environment *env);
        ~AcousticInference();

    public:
        bool initialize(const JsonObject &args, std::string *error) override;

        bool start(const JsonValue &input, std::string *error) override;
        bool stop() override;

        State state() const override;
        JsonObject result() const override;

    protected:
        class Impl;
    };

}

#endif // ACOUSTICINFERENCE_H
