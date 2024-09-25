#include "versionnumber.h"

#include <vector>
#include <sstream>

namespace dsinfer {

    VersionNumber::VersionNumber() {
        m_numbers[0] = 0;
        m_numbers[1] = 0;
        m_numbers[2] = 0;
        m_numbers[3] = 0;
    }

    VersionNumber::VersionNumber(int major, int minor, int patch, int tweak) {
        m_numbers[0] = major;
        m_numbers[1] = minor;
        m_numbers[2] = patch;
        m_numbers[3] = tweak;
    }

    VersionNumber VersionNumber::fromString(const std::string &s) {
        VersionNumber version;
        std::stringstream ss(s);
        std::string segment;

        // Split the string by '.' and convert each segment to int
        int i = 0;
        while (std::getline(ss, segment, '.')) {
            version.m_numbers[i++] = std::stoi(segment);
        }
        return version;
    }


    std::string VersionNumber::toString() const {
        return std::to_string(major()) + "." + std::to_string(minor()) + "." +
               std::to_string(patch()) + "." + std::to_string(tweak());
    }

    bool VersionNumber::operator==(const VersionNumber &rhs) const {
        return major() == rhs.major() && minor() == rhs.minor() && patch() == rhs.patch() &&
               tweak() == rhs.tweak();
    }

    bool VersionNumber::operator!=(const VersionNumber &rhs) const {
        return !(*this == rhs);
    }

    bool VersionNumber::operator<(const VersionNumber &rhs) const {
        if (major() < rhs.major())
            return true;
        if (major() > rhs.major())
            return false;
        if (minor() < rhs.minor())
            return true;
        if (minor() > rhs.minor())
            return false;
        if (patch() < rhs.patch())
            return true;
        if (patch() > rhs.patch())
            return false;
        return tweak() < rhs.tweak();
    }

    bool VersionNumber::operator>(const VersionNumber &rhs) const {
        if (major() > rhs.major())
            return true;
        if (major() < rhs.major())
            return false;
        if (minor() > rhs.minor())
            return true;
        if (minor() < rhs.minor())
            return false;
        if (patch() > rhs.patch())
            return true;
        if (patch() < rhs.patch())
            return false;
        return tweak() > rhs.tweak();
    }

    bool VersionNumber::operator<=(const VersionNumber &rhs) const {
        return !(*this > rhs);
    }

    bool VersionNumber::operator>=(const VersionNumber &rhs) const {
        return !(*this < rhs);
    }

}