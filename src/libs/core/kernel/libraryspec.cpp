#include "libraryspec.h"
#include "libraryspec_p.h"

#include <map>
#include <fstream>
#include <sstream>

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
            for (const auto &spec : it.second) {
                delete spec;
            }
        }
    }

    static LibraryDependency readDependency(const JsonValue &val) {
        if (!val.isObject()) {
            return {};
        }

        std::string id;
        VersionNumber version;
        bool required = true;

        auto obj = val.toObject();
        auto it = obj.find("id");
        if (it == obj.end()) {
            return {};
        }
        id = it->second.toString();
        if (id.empty()) {
            return {};
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
        res.id = id;
        res.version = version;
        return res;
    }

    bool LibrarySpec::Impl::read(const std::filesystem::path &dir,
                                 const std::map<std::string, ContributeRegistry *> &regs,
                                 Error *error) {
        std::string id_;
        VersionNumber version_;
        VersionNumber compatVersion_;
        std::string vendor_;
        std::string copyright_;
        std::string description_;
        std::string url_;
        std::vector<LibraryDependency> dependencies_;

        std::vector<ContributeSpec *> contributes_;

        // Read desc
        JsonObject obj;
        {
            const auto &descPath = dir / _TSTR("desc.json");
            std::ifstream file(descPath);
            if (!file.is_open()) {
                *error = {
                    Error::FileNotFound,
                    formatTextN(R"(failed to open library manifest "%1")", descPath),
                };
                return false;
            }

            std::stringstream ss;
            ss << file.rdbuf();

            std::string error2;
            auto root = JsonValue::fromJson(ss.str(), &error2);
            if (!error2.empty()) {
                *error = {
                    Error::InvalidFormat,
                    formatTextN(R"(invalid library manifest format "%1": %2)", descPath, error2),
                };
                return false;
            }
            if (!root.isObject()) {
                *error = {
                    Error::InvalidFormat,
                    formatTextN(R"(invalid library manifest format "%1")", descPath),
                };
                return false;
            }
            obj = root.toObject();
        }

        // id
        {
            auto it = obj.find("id");
            if (it == obj.end()) {
                *error = {
                    Error::InvalidFormat,
                    R"(missing "id" field in library manifest)",
                };
                return false;
            }
            id_ = it->second.toString();
            if (id_.empty()) {
                *error = {
                    Error::InvalidFormat,
                    R"("id" field has invalid value in library manifest)",
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
                    R"(missing "version" field in library manifest)",
                };
                return false;
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
        // vendor
        {
            auto it = obj.find("vendor");
            if (it != obj.end()) {
                vendor_ = it->second.toString();
            }
        }
        // copyright
        {
            auto it = obj.find("copyright");
            if (it != obj.end()) {
                copyright_ = it->second.toString();
            }
        }
        // description
        {
            auto it = obj.find("description");
            if (it != obj.end()) {
                description_ = it->second.toString();
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
                        R"("dependencies" field has invalid value in library manifest)",
                    };
                    return false;
                }

                for (const auto &item : it->second.toArray()) {
                    auto dep = readDependency(item);
                    if (dep.id.empty()) {
                        *error = {
                            Error::InvalidFormat,
                            R"(unknown data in "dependencies" field in library manifest)",
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
                    auto it2 = regs.find(pair.first);
                    if (it2 == regs.end()) {
                        *error = {
                            Error::FeatureNotSupported,
                            R"(unknown contribute "%1")",
                        };
                        goto out_failed;
                    }

                    const auto &reg = it2->second;
                    auto contribute = reg->parseSpec(fs::canonical(dir), pair.second, error);
                    if (!contribute) {
                        goto out_failed;
                    }
                }

                break;

            out_failed:
                deleteAll(contributes_);
                return false;
            } while (false);

            // Change to loaded state
//            for (const auto &item :)

        }

        id = id_;
        version = version_;
        compatVersion = compatVersion_;
        vendor = vendor_;
        copyright = copyright_;
        description = description_;
        url = url_;
        dependencies = dependencies_;
        return true;
    }

    LibrarySpec::~LibrarySpec() = default;

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

    std::string LibrarySpec::description() const {
        __dsinfer_impl_t;
        return impl.description;
    }

    std::string LibrarySpec::vendor() const {
        __dsinfer_impl_t;
        return impl.vendor;
    }

    std::string LibrarySpec::copyright() const {
        __dsinfer_impl_t;
        return impl.copyright;
    }

    std::string LibrarySpec::url() const {
        __dsinfer_impl_t;
        return impl.url;
    }

    const std::vector<ContributeSpec *> &LibrarySpec::contributes(int type) const {
        __dsinfer_impl_t;
        auto it = impl.contributes.find(type);
        if (it == impl.contributes.end()) {
            static std::vector<ContributeSpec *> _empty;
            return _empty;
        }
        return it->second;
    }

    ContributeSpec *LibrarySpec::contribute(int type, const std::string &id) const {
        return nullptr;
    }

    const std::vector<LibraryDependency> &LibrarySpec::dependencies() const {
        __dsinfer_impl_t;
        return impl.dependencies;
    }

    bool LibrarySpec::hasError() const {
        return false;
    }

    std::string LibrarySpec::errorMessage() const {
        return {};
    }

    Environment *LibrarySpec::env() const {
        return nullptr;
    }

    LibrarySpec::LibrarySpec(Environment *env) : _impl(new Impl(env)) {
    }

}