#ifndef VERSIONNUMBER_H
#define VERSIONNUMBER_H

#include <string>

#include <dsinfer/dsinferglobal.h>

namespace dsinfer {

    class DSINFER_EXPORT VersionNumber {
    public:
        VersionNumber();
        explicit VersionNumber(int major, int minor = 0, int patch = 0, int tweak = 0);

        static VersionNumber fromString(const std::string &s);

    public:
        inline int major() const {
            return m_numbers[0];
        }

        inline int minor() const {
            return m_numbers[1];
        }

        inline int patch() const {
            return m_numbers[2];
        }

        inline int tweak() const {
            return m_numbers[3];
        }

        std::string toString() const;

        bool operator==(const VersionNumber &rhs) const;
        bool operator!=(const VersionNumber &rhs) const;
        bool operator<(const VersionNumber &rhs) const;
        bool operator>(const VersionNumber &rhs) const;
        bool operator<=(const VersionNumber &rhs) const;
        bool operator>=(const VersionNumber &rhs) const;

    private:
        int m_numbers[4];
    };

}

#endif // VERSIONNUMBER_H
