#include "onnxsession.h"

namespace dsinfer {

    OnnxSession::OnnxSession() {
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