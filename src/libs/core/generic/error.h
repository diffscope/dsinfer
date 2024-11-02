#ifndef DSINFER_ERROR_H
#define DSINFER_ERROR_H

#include <string>
#include <utility>

#include <dsinfer/dsinferglobal.h>

namespace dsinfer {

    class DSINFER_EXPORT Error {
    public:
        enum Type {
            NoError = 0,
            InvalidFormat,
            FileNotFound,
            FileDuplicated,
            RecursiveDependency,
            FeatureNotSupported,
            LibraryNotFound,
            SessionError,
        };

        inline Error() : m_type(NoError) {
        }

        inline Error(int type, std::string msg) : m_type(type), m_msg(std::move(msg)) {
        }

        inline Error(int type, const char *msg) : m_type(type), m_msg(msg) {
        }

        inline int type() const {
            return m_type;
        }

        inline bool ok() const {
            return m_type == NoError;
        }

        inline std::string message() const {
            return m_msg;
        }

        inline const char *what() const {
            return m_msg.c_str();
        }

    protected:
        std::string m_msg;
        int m_type;
    };

}

#endif // DSINFER_ERROR_H
