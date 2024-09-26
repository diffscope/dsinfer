#include "inferencesession.h"

namespace dsinfer {

    class InferenceSession::Impl {
    public:
        explicit Impl(Environment *env) : env(env) {
        }

        Environment *env;

        int64_t id = 0;
        bool running = false;
    };

    InferenceSession::InferenceSession(Environment *env) : _impl(new Impl(env)) {
    }

    InferenceSession::~InferenceSession() = default;

    bool InferenceSession::open(const std::filesystem::path &path, const JsonObject &args,
                                Error *error) {
        __dsinfer_impl_t;
        return false;
    }

    void InferenceSession::close() {
        __dsinfer_impl_t;
    }

    int64_t InferenceSession::id() const {
        __dsinfer_impl_t;
        return impl.id;
    }

    bool InferenceSession::isRunning() const {
        __dsinfer_impl_t;
        return impl.running;
    }

    Environment *InferenceSession::env() const {
        __dsinfer_impl_t;
        return impl.env;
    }

}