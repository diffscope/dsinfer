#include <cstring>
#include <stdexcept>

#include "dsinfer_global.h"
#include "dsinfer_capi.h"
#include "core/common.h"

// See ONNXRuntime's implementation of OrtStatus (https://github.com/microsoft/onnxruntime)

namespace {
    inline DSINFER_Status *new_status(size_t clen) {
        auto buf = new (std::nothrow) uint8_t[sizeof(DSINFER_Status) + clen];
        if (buf == nullptr) {
            return nullptr;
        }
        return new (buf) DSINFER_Status;
    }
}  // namespace

DSINFER_EXPORT DSINFER_Status *create_status(int type, int code, const char *message) {
    size_t clen(nullptr == message ? 0 : strnlen(message, dsinfer::kMaxStrLen));
    DSINFER_Status *p = new_status(clen);
    if (p == nullptr) {
        return nullptr;
    }
    p->type = type;
    p->code = code;
    std::memcpy(p->message, message, clen);
    p->message[clen] = '\0';
    return p;
}

DSINFER_EXPORT void release_status(DSINFER_Status *status) {
    delete[] reinterpret_cast<uint8_t *>(status);
}
