#ifndef DSINFER_CXXAPI_H
#define DSINFER_CXXAPI_H

#include <cstddef>
#include <string>
#include <dsinfer/dsinfer_capi.h>

namespace dsinfer {
    class DSINFER_EXPORT Status {
    public:
        explicit Status(std::nullptr_t);
        Status(int type, int code, const char *message);
        Status(int type, int code, const std::string &message);
        /**
             * @brief Takes ownership of existing DSINFER_Status pointer.
         */
        explicit Status(DSINFER_Status *status);
        ~Status();
        Status(const Status &) = delete;
        Status(Status &&) noexcept;

        [[nodiscard]] int type() const;
        [[nodiscard]] int code() const;
        [[nodiscard]] std::string message() const;

        /**
             * @brief Relinquishes ownership of the contained DSINFER_Status pointer.
         */
        DSINFER_Status *release();
    private:
        DSINFER_Status *m_status;
    };
}

#endif // DSINFER_CXXAPI_H
