#include <cstring>
#include <stdexcept>

#include <dsinfer/dsinfer_global.h>
#include <dsinfer/dsinfer_capi.h>
#include <dsinfer/dsinfer_cxxapi.h>
#include <dsinfer/common.h>


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

namespace dsinfer {
    constexpr int kStatusDefaultType = ET_Invalid;
    constexpr int kStatusDefaultCode = EC_Invalid;

    Status::Status(std::nullptr_t) {
        m_status = nullptr;
    }

    Status::Status(int type, int code, const char *message) {
        m_status = create_status(type, code, message);
    }

    Status::Status(int type, int code, const std::string &message) {
        m_status = create_status(type, code, message.c_str());
    }

    Status::Status(DSINFER_Status *status) {
        m_status = status;
    }

    Status::Status(Status &&other) noexcept : m_status(other.release()) {}

    Status::~Status() {
        if (m_status) {
            release_status(m_status);
            m_status = nullptr;
        }
    }

    DSINFER_Status *Status::release() {
        auto p = m_status;
        m_status = nullptr;  // Relinquish ownership
        return p;
    }

    int Status::type() const {
        if (!m_status) {
            return kStatusDefaultType;
        }
        return m_status->type;
    }

    int Status::code() const {
        if (!m_status) {
            return kStatusDefaultCode;
        }
        return m_status->code;
    }

    std::string Status::message() const {
        if (!m_status) {
            return "";
        }
        return m_status->message;
    }
} // namespace dsinfer
