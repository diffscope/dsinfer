#ifndef ONNXSESSION_P_H
#define ONNXSESSION_P_H

#include "onnxsession.h"
#include "internal/session.h"
#include "internal/valuemap.h"

namespace dsinfer {
    class OnnxSession::Impl {
    public:
        int64_t sessionId = 0;
        onnxdriver::Session session;
    };
}

#endif // ONNXSESSION_P_H