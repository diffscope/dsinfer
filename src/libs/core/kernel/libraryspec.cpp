#include "libraryspec.h"
#include "libraryspec_p.h"

#include <map>
#include <fstream>
#include <sstream>
#include <unordered_set>

#include "contributeregistry.h"
#include "contributespec.h"
#include "format.h"
#include "algorithms.h"

namespace fs = std::filesystem;

namespace dsinfer {

    bool LibraryDependency::operator==(const LibraryDependency &other) const {
        return id == other.id && version == other.version;
    }

    LibrarySpec::Impl::~Impl() {
        for (const auto &it : std::as_const(contributes)) {
            for (const auto &it2 : it.second) {
                delete it2.second;
            }
        }
    }

    static bool readDependency(const JsonValue &val, LibraryDependency *out,
                               std::string *errorMessage) {
        if (val.isString()) {
            auto identifier = ContributeIdentifier::fromString(val.toString());
            if (!identifier.library().empty() && !identifier.version().isEmpty() &&
                identifier.id().empty()) {
                LibraryDependency res;
                res.id = identifier.library();
                res.version = identifier.version();
                *out = res;
                return true;
            }
            *errorMessage = R"(invalid id)";
            return false;
        }

        if (!val.isObject()) {
            *errorMessage = R"(not an object)";
            return false;
        }

        std::string id;
        VersionNumber version;
        bool required = true;

        auto obj = val.toObject();
        auto it = obj.find("id");
        if (it == obj.end()) {
            *errorMessage = R"(missing "id" field)";
            return false;
        }
        id = it->second.toString();
        if (id.empty()) {
            *errorMessage = R"(invalid id)";
            return false;
        }

        it = obj.find("version");
        if (it != obj.end()) {
            version = VersionNumber::fromString(it->second.toString());
        }

        it = obj.find("required");
        if (it != obj.end() && !it->second.toBool()) {
            required = false;
        }

        LibraryDependency res(required);
        res.id = std::move(id);
        res.version = version;
        *out = res;
        return true;
    }

    bool LibrarySpec::Impl::parse(const std::filesystem::path &dir,
                                  const std::map<std::string, ContributeRegistry *> &regs,
                                  std::vector<ContributeSpec *> *outContributes, Error *error) {
        __dsinfer_decl_t;

        std::string id_;
        VersionNumber version_;
        VersionNumber compatVersion_;
        DisplayText vendor_;
        DisplayText copyright_;
        DisplayText description_;
        fs::path readme_;
        std::string url_;
        std::vector<LibraryDependency> dependencies_;

        std::vector<ContributeSpec *> contributes_;

        // Read desc
        JsonObject obj;
        if (!readDesc(dir, &obj, error)) {
            return false;
        }

        auto canonicalDir = fs::canonical(dir);
        const auto &descPath = canonicalDir / _TSTR("desc.json");

        // id
        {
            auto it = obj.find("id");
            if (it == obj.end()) {
                *error = {
                    Error::InvalidFormat,
                    formatTextN(R"(%1: missing "id" field)", descPath),
                };
                return false;
            }
            id_ = it->second.toString();
            if (!ContributeIdentifier::isValidId(id_)) {
                *error = {
                    Error::InvalidFormat,
                    formatTextN(R"(%1: "id" field has invalid value)", descPath),
                };
                return false;
            }
        }
        // version
        {
            auto it = obj.find("version");
            if (it == obj.end()) {
                *error = {
                    Error::InvalidFormat,
                    formatTextN(R"(%1: missing "version" field)", descPath),
                };
                return false;
            }
            version_ = VersionNumber::fromString(it->second.toString());
            if (version_.isEmpty()) {
                *error = {
                    Error::InvalidFormat,
                    formatTextN(R"(%1: invalid version)", descPath),
                };
                return false;
            }
        }
        // compatVersion
        {
            auto it = obj.find("compatVersion");
            if (it != obj.end()) {
                compatVersion_ = VersionNumber::fromString(it->second.toString());
            } else {
                compatVersion_ = version_;
            }
        }
        // vendor
        {
            auto it = obj.find("vendor");
            if (it != obj.end()) {
                vendor_ = it->second;
            }
        }
        // copyright
        {
            auto it = obj.find("copyright");
            if (it != obj.end()) {
                copyright_ = it->second;
            }
        }
        // description
        {
            auto it = obj.find("description");
            if (it != obj.end()) {
                description_ = it->second;
            }
        }
        // readme
        {
            auto it = obj.find("readme");
            if (it != obj.end()) {
                readme_ = pathFromString(it->second.toString());
            }
        }
        // url
        {
            auto it = obj.find("url");
            if (it != obj.end()) {
                url_ = it->second.toString();
            }
        }
        // dependencies
        {
            auto it = obj.find("dependencies");
            if (it != obj.end()) {
                if (!it->second.isArray()) {
                    *error = {
                        Error::InvalidFormat,
                        formatTextN(R"(%1: "dependencies" field has invalid value)", descPath),
                    };
                    return false;
                }

                for (const auto &item : it->second.toArray()) {
                    std::string errorMessage;
                    LibraryDependency dep;
                    if (!readDependency(item, &dep, &errorMessage)) {
                        *error = {
                            Error::InvalidFormat,
                            formatTextN(R"(%1: invalid "dependencies" field entry %2: %3)",
                                        descPath, dependencies_.size() + 1, errorMessage),
                        };
                        return false;
                    }
                    dependencies_.push_back(dep);
                }
            }
        }
        // contributes
        {
            auto it = obj.find("contributes");
            if (it != obj.end()) {
                if (!it->second.isObject()) {
                    *error = {
                        Error::InvalidFormat,
                        R"("contributes" field has invalid value in library manifest)",
                    };
                    return false;
                }
            }

            do {
                for (const auto &pair : it->second.toObject()) {
                    const auto &contributeKey = pair.first;
                    auto it2 = regs.find(contributeKey);
                    if (it2 == regs.end()) {
                        *error = {
                            Error::FeatureNotSupported,
                            formatTextN(R"(unknown contribute "%1")", contributeKey),
                        };
                        goto out_failed;
                    }

                    const auto &reg = it2->second;
                    if (!pair.second.isArray()) {
                        *error = {
                            Error::InvalidFormat,
                            formatTextN(
                                R"(contribute "%1" field has invalid value in library manifest)",
                                contributeKey),
                        };
                        goto out_failed;
                    }

                    std::unordered_set<std::string> idSet;
                    for (const auto &item : pair.second.toArray()) {
                        auto contribute = reg->parseSpec(canonicalDir, item, error);
                        if (!contribute) {
                            goto out_failed;
                        }
                        contributes_.push_back(contribute);

                        // Check id
                        const auto &contributeId = contribute->id();
                        if (idSet.count(contributeId)) {
                            *error = {
                                Error::InvalidFormat,
                                formatTextN(R"(contribute "%1" object has duplicated id "%2")",
                                            pair.first, contributeId),
                            };
                            goto out_failed;
                        }
                        idSet.emplace(contributeId);
                    }
                }

                break;

            out_failed:
                deleteAll(contributes_);
                return false;
            } while (false);
        }

        path = canonicalDir;
        id = std::move(id_);
        version = version_;
        compatVersion = compatVersion_;
        vendor = std::move(vendor_);
        copyright = std::move(copyright_);
        description = std::move(description_);
        readme = std::move(readme_);
        url = std::move(url_);
        dependencies = std::move(dependencies_);
        *outContributes = std::move(contributes_);
        return true;
    }

    bool LibrarySpec::Impl::readDesc(const std::filesystem::path &dir, JsonObject *out,
                                     dsinfer::Error *error) {
        const auto &descPath = dir / _TSTR("desc.json");
        std::ifstream file(descPath);
        if (!file.is_open()) {
            *error = {
                Error::FileNotFound,
                formatTextN(R"("%1": failed to open library manifest)", descPath),
            };
            return false;
        }

        std::stringstream ss;
        ss << file.rdbuf();

        std::string error2;
        auto root = JsonValue::fromJson(ss.str(), true, &error2);
        if (!error2.empty()) {
            *error = {
                Error::InvalidFormat,
                formatTextN(R"("%1": invalid library manifest format: %2)", descPath, error2),
            };
            return false;
        }
        if (!root.isObject()) {
            *error = {
                Error::InvalidFormat,
                formatTextN(R"("%1": invalid library manifest format)", descPath),
            };
            return false;
        }
        *out = root.toObject();
        return true;
    }

    LibrarySpec::~LibrarySpec() = default;

    LibrarySpec::LibrarySpec(LibrarySpec &&other) noexcept {
        std::swap(_impl, other._impl);
    }

    LibrarySpec &LibrarySpec::operator=(LibrarySpec &&other) noexcept {
        if (this == &other)
            return *this;
        std::swap(_impl, other._impl);
        return *this;
    }

    std::filesystem::path LibrarySpec::path() const {
        __dsinfer_impl_t;
        return impl.path;
    }

    std::string LibrarySpec::id() const {
        __dsinfer_impl_t;
        return impl.id;
    }

    VersionNumber LibrarySpec::version() const {
        __dsinfer_impl_t;
        return impl.version;
    }

    VersionNumber LibrarySpec::compatVersion() const {
        __dsinfer_impl_t;
        return impl.compatVersion;
    }

    DisplayText LibrarySpec::description() const {
        __dsinfer_impl_t;
        return impl.description;
    }

    DisplayText LibrarySpec::vendor() const {
        __dsinfer_impl_t;
        return impl.vendor;
    }

    DisplayText LibrarySpec::copyright() const {
        __dsinfer_impl_t;
        return impl.copyright;
    }

    std::filesystem::path LibrarySpec::readme() const {
        __dsinfer_impl_t;
        return impl.readme;
    }

    std::string LibrarySpec::url() const {
        __dsinfer_impl_t;
        return impl.url;
    }

    std::vector<ContributeSpec *> LibrarySpec::contributes(int type) const {
        __dsinfer_impl_t;
        auto it = impl.contributes.find(type);
        if (it == impl.contributes.end()) {
            return {};
        }

        std::vector<ContributeSpec *> res;
        const auto &map2 = it->second;
        res.reserve(map2.size());
        for (const auto &pair : std::as_const(map2)) {
            res.push_back(pair.second);
        }
        return res;
    }

    ContributeSpec *LibrarySpec::contribute(int type, const std::string &id) const {
        __dsinfer_impl_t;
        auto it = impl.contributes.find(type);
        if (it == impl.contributes.end()) {
            return nullptr;
        }

        const auto &map2 = it->second;
        auto it2 = map2.find(id);
        if (it2 == map2.end()) {
            return nullptr;
        }
        return it2->second;
    }

    const std::vector<LibraryDependency> &LibrarySpec::dependencies() const {
        __dsinfer_impl_t;
        return impl.dependencies;
    }

    Error LibrarySpec::error() const {
        __dsinfer_impl_t;
        return impl.err;
    }

    bool LibrarySpec::isLoaded() const {
        __dsinfer_impl_t;
        return impl.loaded;
    }

    Environment *LibrarySpec::env() const {
        __dsinfer_impl_t;
        return impl.env;
    }

    std::filesystem::path
        LibrarySpec::searchLibrary(const std::vector<std::filesystem::path> &paths,
                                   const std::string &id, const VersionNumber &version,
                                   bool precise) {
        try {
            for (const auto &path : paths) {
                for (const auto &entry : fs::directory_iterator(path)) {
                    const auto filename = entry.path().filename();
                    if (!entry.is_directory()) {
                        continue;
                    }

                    JsonObject obj;
                    Error error;
                    if (!Impl::readDesc(entry.path(), &obj, &error)) {
                        continue;
                    }

                    // Search id, version, compatVersion
                    std::string id_;
                    VersionNumber version_;
                    VersionNumber compatVersion_;

                    // id
                    {
                        auto it = obj.find("id");
                        if (it == obj.end()) {
                            continue;
                        }
                        id_ = it->second.toString();
                        if (!ContributeIdentifier::isValidId(id_)) {
                            continue;
                        }
                    }
                    // version
                    {
                        auto it = obj.find("version");
                        if (it == obj.end()) {
                            continue;
                        }
                        version_ = VersionNumber::fromString(it->second.toString());
                    }
                    // compatVersion
                    {
                        auto it = obj.find("compatVersion");
                        if (it != obj.end()) {
                            compatVersion_ = VersionNumber::fromString(it->second.toString());
                        } else {
                            compatVersion_ = version_;
                        }
                    }

                    // Check
                    if (id_ != id) {
                        continue;
                    }
                    if (precise) {
                        if (version_ != version) {
                            continue;
                        }
                    } else {
                        if (compatVersion_ > version) {
                            continue;
                        }
                    }
                    return fs::canonical(entry.path());
                }
            }
        } catch (...) {
        }
        return {};
    }

    LibrarySpec::LibrarySpec(Environment *env) : _impl(new Impl(this, env)) {
    }

}