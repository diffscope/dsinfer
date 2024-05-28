#ifndef DSINFER_SESSIONMANAGER_H
#define DSINFER_SESSIONMANAGER_H

#include <cstdint>
#include <memory>
#include <dsinfer/dsinfer_global.h>
#include <dsinfer/sharedsession.h>

namespace dsinfer {
    // Use auto-increment ID starting from 1.
    // 0 means the ID is invalid.
    using SessionHandle = size_t;

    class DSINFER_EXPORT SessionManager {
    public:
        SessionManager();
        ~SessionManager();

        SessionHandle create(const std::filesystem::path &path);
        SharedSession get(SessionHandle id);
        bool remove(SessionHandle id);
        size_t count() const;
    protected:
        class Impl;
        std::unique_ptr<Impl> _impl;

        DSINFER_DISABLE_COPY(SessionManager)
    };
}

#endif