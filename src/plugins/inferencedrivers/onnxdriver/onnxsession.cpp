#include "onnxsession.h"
#include "onnxsession_p.h"

#include "internal/onnxdriver_common.h"
#include "internal/onnxdriver_logger.h"
#include "internal/session.h"
#include "internal/idutil.h"

namespace dsinfer {

    static IdManager<OnnxSession> &idManager() {
        static IdManager<OnnxSession> manager;
        return manager;
    }

    OnnxSession::OnnxSession() : _impl(std::make_unique<Impl>()) {
        __stdc_impl_t;
        auto sessionId = idManager().add(this);
        impl.sessionId = sessionId;
        onnxdriver_log().debug("OnnxSession [%1] - new session created", sessionId);
    }

    OnnxSession::~OnnxSession() {
        __stdc_impl_t;

        // Ensure close
        std::ignore = impl.session.close();
        idManager().remove(impl.sessionId);
    }

    OnnxSession *OnnxSession::getSession(int64_t sessionId) {
        return idManager().get(sessionId);
    }

    bool OnnxSession::open(const std::filesystem::path &path, const JsonValue &args, Error *error) {
        __stdc_impl_t;
        int hints = 0;
        auto obj = args.toObject();
        if (auto it = obj.find("useCpuHint"); it != obj.end()) {
            if (it->second.isBool() && it->second.toBool()) {
                hints |= onnxdriver::SH_PreferCPUHint;
            }
        }
        return impl.session.open(path, hints, error);
    }

    bool OnnxSession::isOpen() const {
        __stdc_impl_t;
        return impl.session.isOpen();
    }

    bool OnnxSession::close(Error *error) {
        __stdc_impl_t;
        return impl.session.close();
    }

    int64_t OnnxSession::id() const {
        __stdc_impl_t;
        return impl.sessionId;
    }

    bool OnnxSession::isRunning() const {
        __stdc_impl_t;
        // TODO: implement
        return false;
    }

}