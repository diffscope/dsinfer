#include "onnxsession.h"

#include <onnxruntime_cxx_api.h>

namespace dsinfer {

    class OnnxSession::Impl {
    public:
        Impl() {
        }
    };

    OnnxSession::OnnxSession() : _impl(std::make_unique<Impl>()) {
    }

    OnnxSession::~OnnxSession() {
    }

    bool OnnxSession::open(const std::filesystem::path &path, const JsonObject &args,
                           Error *error) {
        return false;
    }

    bool OnnxSession::close(Error *error) {
        return false;
    }

    int64_t OnnxSession::id() const {
        return reinterpret_cast<int64_t>(this);
    }

    bool OnnxSession::isRunning() const {
        return false;
    }

}