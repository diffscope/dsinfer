#include "sharedsession.h"

#include <dsinfer/session.h>
#include "session_p.h"

namespace dsinfer {

    SharedSession::SharedSession(Session &session) : m_session(&session) {
        if (m_session) {
            m_session->d->load();
        }
    }

    SharedSession::SharedSession(std::nullptr_t) : m_session(nullptr) {}

    SharedSession::~SharedSession() {
        if (m_session) {
            m_session->d->free();
        }
    }

    SharedSession::SharedSession(const SharedSession &other) : m_session(other.m_session) {
        if (m_session) {
            m_session->d->load();
        }
    }

    SharedSession::SharedSession(SharedSession &&other) noexcept : m_session(other.m_session) {
        other.m_session = nullptr;
    }

    SharedSession &SharedSession::operator=(const SharedSession &other) {
        if (this == &other) {
            return *this;
        }
        if (m_session != other.m_session) {
            if (m_session) {
                m_session->d->free();
            }
            m_session = other.m_session;
        }
        m_session->d->load();
        return *this;
    }

    SharedSession &SharedSession::operator=(SharedSession &&other) noexcept {
        if (this == &other) {
            return *this;
        }
        if (m_session != other.m_session) {
            if (m_session) {
                m_session->d->free();
            }
            m_session = other.m_session;
        }
        other.m_session = nullptr;
        return *this;
    }


    Session *SharedSession::get() {
        return m_session;
    }

    Session *SharedSession::operator->() {
        return m_session;
    }

    Session &SharedSession::operator*() {
        return *m_session;
    }

    SharedSession::operator bool() const {
        return m_session != nullptr;
    }
}
