#include "onnxsession.h"
#include "internal/session.h"

#include <onnxruntime_cxx_api.h>

namespace dsinfer {

    class OnnxSession::Impl {
    public:
        Impl() {
        }

        onnxdriver::Session session;
    };

    OnnxSession::OnnxSession() : _impl(std::make_unique<Impl>()) {
    }

    OnnxSession::~OnnxSession() {
    }

    bool OnnxSession::open(const std::filesystem::path &path, const JsonObject &args,
                           Error *error) {
        __dsinfer_impl_t;
        bool useCpuHint = false;
        if (auto it = args.find("useCpuHint"); it != args.end()) {
            if (it->second.isBool()) {
                useCpuHint = it->second.toBool();
            }
        }

        return impl.session.open(path, useCpuHint, error);
    }

    bool OnnxSession::close(Error *error) {
        __dsinfer_impl_t;
        return impl.session.close();
    }

    int64_t OnnxSession::id() const {
        return reinterpret_cast<int64_t>(this);
    }

    bool OnnxSession::isRunning() const {
        __dsinfer_impl_t;
        return impl.session.isOpen();
    }

}