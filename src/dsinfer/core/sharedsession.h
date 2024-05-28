#ifndef DSINFER_SHAREDSESSION_H
#define DSINFER_SHAREDSESSION_H

#include <cstddef>
#include <dsinfer/session.h>

namespace dsinfer {
    // SharedSession is similar to a shared pointer
    class DSINFER_EXPORT SharedSession {
    public:
        explicit SharedSession(Session &session);
        explicit SharedSession(std::nullptr_t);
        SharedSession(const SharedSession &other);
        SharedSession(SharedSession &&other) noexcept;
        SharedSession &operator=(const SharedSession &other);
        SharedSession &operator=(SharedSession &&other) noexcept;
        ~SharedSession();
        Session *operator->();
        Session &operator*();
        Session *get();
        explicit operator bool() const;
    private:
        Session *m_session;
    };
}
#endif