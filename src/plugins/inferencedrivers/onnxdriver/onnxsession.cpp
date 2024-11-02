#include "onnxsession.h"

#include "internal/onnxdriver_logger.h"
#include "internal/session.h"
#include "internal/idutil.h"

namespace dsinfer {

    static IdManager<OnnxSession> &idManager() {
        static IdManager<OnnxSession> manager;
        return manager;
    }

    class OnnxSession::Impl {
    public:
        Impl() {
        }

        int64_t sessionId = 0;
        onnxdriver::Session session;
    };

    OnnxSession::OnnxSession() : _impl(std::make_unique<Impl>()) {
        __dsinfer_impl_t;
        auto sessionId = idManager().add(this);
        impl.sessionId = sessionId;
        onnxdriver_log().debug("OnnxSession [%1] - new session created", sessionId);
    }

    OnnxSession::~OnnxSession() {
        __dsinfer_impl_t;
        idManager().remove(impl.sessionId);
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
        __dsinfer_impl_t;
        return impl.sessionId;
    }

    OnnxSession *OnnxSession::getSession(int64_t sessionId) {
        return idManager().get(sessionId);
    }

    bool OnnxSession::isRunning() const {
        __dsinfer_impl_t;
        return impl.session.isOpen();
    }

}