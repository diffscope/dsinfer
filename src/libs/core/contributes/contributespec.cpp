#include "contributespec.h"
#include "contributespec_p.h"

#include <regex>

#include "format.h"

namespace dsinfer {

    std::string ContributeIdentifier::toString() const {
        if (m_library.empty()) {
            return m_id;
        }
        if (m_version.isEmpty()) {
            if (m_id.empty()) {
                return m_library;
            }
            return formatTextN("%1/%2", m_library, m_id);
        }
        if (m_id.empty()) {
            return formatTextN("%1[%2]", m_library, m_version.toString());
        }
        return formatTextN("%1[%2]/%3", m_library, m_version.toString(), m_id);
    }

    ContributeIdentifier ContributeIdentifier::fromString(const std::string &token) {
        if (token.empty()) {
            return {};
        }

        // A single regex to handle all cases: id/sid, id[version]/sid, idToken/sid, and sid
        static std::regex pattern(R"((\w+)(\[(\d+(\.\d+){0,3})\])?(\/(\w+))?)");
        std::smatch matches;
        if (std::regex_match(token, matches, pattern)) {
            std::string id = matches[1].str();
            std::string ver = matches[3].matched ? matches[3].str() : std::string();
            std::string sid = matches[5].matched ? matches[5].str() : std::string();
            if (!ver.empty()) {
                return {id, VersionNumber::fromString(ver), sid};
            }
            if (!sid.empty()) {
                return {id, sid};
            }
            return {id};
        }
        return {};
    }


    bool ContributeSpec::Impl::read(const std::filesystem::path &basePath, const JsonObject &obj,
                                    Error *error) {
        return false;
    }

    ContributeSpec::~ContributeSpec() = default;

    int ContributeSpec::type() const {
        __dsinfer_impl_t;
        return impl.type;
    }

    ContributeSpec::State ContributeSpec::state() const {
        __dsinfer_impl_t;
        return impl.state;
    }

    LibrarySpec *ContributeSpec::parent() const {
        __dsinfer_impl_t;
        return impl.parent;
    }

    std::string ContributeSpec::id() const {
        __dsinfer_impl_t;
        return impl.id;
    }

    ContributeSpec::ContributeSpec(Impl &impl) : _impl(&impl) {
    }

}