#include "inferencesession.h"

#include "inferenceregistry.h"
#include "inferencespec.h"

namespace dsinfer {

    class InferenceSession::Impl {
    public:
        explicit Impl(Environment *env) : env(env) {
            reg = env->registry(ContributeSpec::Inference)->cast<InferenceRegistry>();
        }

        Environment *env;
        InferenceRegistry *reg;

        int64_t id = 0;
    };

    InferenceSession::InferenceSession(Environment *env) : _impl(new Impl(env)) {
    }

    InferenceSession::~InferenceSession() {
        Error err;
        close(&err);
    }

    bool InferenceSession::open(const std::filesystem::path &path, const JsonObject &args,
                                Error *error) {
        __dsinfer_impl_t;
        auto id = impl.reg->driver()->sessionCreate(path, args, error);
        if (id == 0) {
            return false;
        }
        impl.id = id;
        return true;
    }

    bool InferenceSession::close(Error *error) {
        __dsinfer_impl_t;

        if (impl.id == 0) {
            return true;
        }
        return impl.reg->driver()->sessionDestroy(impl.id, error);
    }

    int64_t InferenceSession::id() const {
        __dsinfer_impl_t;
        return impl.id;
    }

    bool InferenceSession::isRunning() const {
        __dsinfer_impl_t;
        return impl.reg->driver()->sessionRunning(impl.id);
    }

    Environment *InferenceSession::env() const {
        __dsinfer_impl_t;
        return impl.env;
    }

}