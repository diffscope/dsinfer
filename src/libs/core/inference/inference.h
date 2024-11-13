#ifndef INFERENCE_H
#define INFERENCE_H

#include <functional>

#include <dsinfer/jsonvalue.h>
#include <dsinfer/environment.h>

namespace dsinfer {

    class InferenceSpec;

    class DSINFER_EXPORT Inference {
    public:
        explicit Inference(const InferenceSpec *env);
        virtual ~Inference();

        enum State {
            Idle,
            Running,
            Failed,
            Terminated,
        };

    public:
        virtual bool initialize(const JsonValue &args, Error *error) = 0;

        virtual bool start(const JsonValue &input, Error *error) = 0;
        virtual bool startAsync(const JsonValue &input,
                                const std::function<void(const JsonValue &, const Error &)> &callback,
                                Error *error) = 0;
        virtual bool stop() = 0;

        virtual State state() const = 0;
        virtual JsonValue result() const = 0;

    public:
        const InferenceSpec *spec() const;
        Environment *env() const;

    protected:
        class Impl;
        std::unique_ptr<Impl> _impl;
        explicit Inference(Impl &impl);
    };

}

#endif // INFERENCE_H
