#ifndef INFERENCE_H
#define INFERENCE_H

#include <dsinfer/jsonvalue.h>
#include <dsinfer/environment.h>

namespace dsinfer {

    class DSINFER_EXPORT Inference {
    public:
        explicit Inference(Environment *env);
        virtual ~Inference();

        enum State {
            Idle,
            Running,
            Error,
            Terminated,
        };

    public:
        virtual bool initialize(const JsonObject &args, std::string *error) = 0;

        virtual bool start(const JsonValue &input, std::string *error) = 0;
        virtual bool stop() = 0;

        virtual State state() const = 0;
        virtual JsonObject result() const = 0;

    public:
        Environment *env() const;

    protected:
        class Impl;
        std::unique_ptr<Impl> _impl;
        explicit Inference(Impl &impl);
    };

}

#endif // INFERENCE_H
