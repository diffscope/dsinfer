#include "singerspec.h"
#include "singerspec_p.h"

#include <stdcorelib/format.h>

#include <fstream>

namespace fs = std::filesystem;

namespace dsinfer {

    static bool readSingerImport(const JsonValue &val, SingerImport *out,
                                 std::string *errorMessage) {
        if (val.isString()) {
            auto inference = ContributeIdentifier::fromString(val.toString());
            if (inference.id().empty()) {
                *errorMessage = R"(invalid id)";
                return false;
            }
            SingerImport res;
            res.inference = inference;
            *out = std::move(res);
            return true;
        }
        if (!val.isObject()) {
            *errorMessage = R"(invalid data type)";
            return false;
        }
        auto obj = val.toObject();
        auto it = obj.find("id");
        if (it == obj.end()) {
            *errorMessage = R"(missing "id" field)";
            return false;
        }
        auto inference = ContributeIdentifier::fromString(it->second.toString());
        SingerImport res;
        res.inference = inference;

        // roles
        // it = obj.find("roles");
        // if (it == obj.end() || !it->second.isArray()) {
        //     *errorMessage = R"(missing "roles" field)";
        //     return false;
        // }
        // const auto &arr = it->second.toArray();
        // for (const auto &item : arr) {
        //     if (!item.isString()) {
        //         *errorMessage =
        //             stdc::formatTextN(R"(invalid item in roles field entry %1)", res.roles.size() + 1);
        //         return false;
        //     }
        //     auto role = item.toString();
        //     if (role.empty()) {
        //         *errorMessage =
        //             stdc::formatTextN(R"(empty item in roles field entry %1)", res.roles.size() + 1);
        //         return false;
        //     }
        //     res.roles.emplace_back(role);
        // }

        // options
        it = obj.find("options");
        if (it != obj.end()) {
            res.options = it->second;
        }
        *out = std::move(res);
        return true;
    }

    bool SingerSpec::Impl::read(const std::filesystem::path &basePath, const JsonObject &obj,
                                Error *error) {
        fs::path configPath;
        VersionNumber fmtVersion_;
        std::string id_;
        std::string model_;

        DisplayText name_;

        fs::path avatar_;
        fs::path background_;
        fs::path demoAudio_;

        std::vector<SingerImport> imports_;
        JsonObject configuration_;

        // Parse desc
        {
            // id
            auto it = obj.find("id");
            if (it == obj.end()) {
                *error = {
                    Error::InvalidFormat,
                    R"(missing "id" field in singer contribute field)",
                };
                return false;
            }
            id_ = it->second.toString();
            if (!ContributeIdentifier::isValidId(id_)) {
                *error = {
                    Error::InvalidFormat,
                    R"("id" field has invalid value in singer contribute field)",
                };
                return false;
            }

            // model
            it = obj.find("model");
            if (it == obj.end()) {
                *error = {
                    Error::InvalidFormat,
                    R"(missing "model" field in singer contribute field)",
                };
                return false;
            }
            model_ = it->second.toString();
            if (model_.empty()) {
                *error = {
                    Error::InvalidFormat,
                    R"("model" field has invalid value in singer contribute field)",
                };
                return false;
            }

            // path
            it = obj.find("path");
            if (it == obj.end()) {
                *error = {
                    Error::InvalidFormat,
                    R"(missing "path" field in singer contribute field)",
                };
                return false;
            }

            std::string configPathString = it->second.toString();
            if (configPathString.empty()) {
                *error = {
                    Error::InvalidFormat,
                    R"("path" field has invalid value in singer contribute field)",
                };
                return false;
            }

            configPath = stdc::utf8ToPath(configPathString);
            if (configPath.is_relative()) {
                configPath = basePath / configPath;
            }
        }

        // Read configuration
        JsonObject configObj;
        {
            std::ifstream file(configPath);
            if (!file.is_open()) {
                *error = {
                    Error::FileNotFound,
                    stdc::formatTextN(R"(%1: failed to open singer manifest)", configPath),
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
                    stdc::formatTextN(R"(%1: invalid singer manifest format: %2)", configPath, error2),
                };
                return false;
            }
            if (!root.isObject()) {
                *error = {
                    Error::InvalidFormat,
                    stdc::formatTextN(R"(%1: invalid singer manifest format)", configPath),
                };
                return false;
            }
            configObj = root.toObject();
        }

        // Get attributes
        // $version
        {
            auto it = configObj.find("$version");
            if (it == configObj.end()) {
                *error = {
                    Error::InvalidFormat,
                    stdc::formatTextN(R"(%1: missing "$version" field)", configPath),
                };
                return false;
            }
            fmtVersion_ = VersionNumber::fromString(it->second.toString());
            if (fmtVersion_ > VersionNumber(1)) {
                *error = {
                    Error::FeatureNotSupported,
                    stdc::formatTextN(R"(%1: format version "%1" is not supported)",
                                fmtVersion_.toString()),
                };
                return false;
            }
        }
        // name
        {
            auto it = configObj.find("name");
            if (it != configObj.end()) {
                name_ = it->second;
            }
            if (name_.isEmpty()) {
                name_ = id_;
            }
        }
        // avatar
        {
            auto it = configObj.find("avatar");
            if (it != configObj.end()) {
                avatar_ = stdc::utf8ToPath(it->second.toString());
            }
        }
        // background
        {
            auto it = configObj.find("background");
            if (it != configObj.end()) {
                background_ = stdc::utf8ToPath(it->second.toString());
            }
        }
        // demoAudio
        {
            auto it = configObj.find("demoAudio");
            if (it != configObj.end()) {
                demoAudio_ = stdc::utf8ToPath(it->second.toString());
            }
        }
        // imports
        {
            auto it = configObj.find("imports");
            if (it != configObj.end()) {
                if (!it->second.isArray()) {
                    *error = {
                        Error::InvalidFormat,
                        stdc::formatTextN(R"(%1: "imports" field has invalid value)", configPath),
                    };
                    return false;
                }

                for (const auto &item : it->second.toArray()) {
                    SingerImport singerImport;
                    std::string errorMessage;
                    if (!readSingerImport(item, &singerImport, &errorMessage)) {
                        *error = {
                            Error::InvalidFormat,
                            stdc::formatTextN(R"(%1: invalid "imports" field entry %2: %3)", configPath,
                                        imports_.size() + 1, errorMessage),
                        };
                        return false;
                    }
                    imports_.push_back(singerImport);
                }
            }
        }
        // configuration
        {
            auto it = configObj.find("configuration");
            if (it != configObj.end()) {
                if (!it->second.isObject()) {
                    *error = {
                        Error::InvalidFormat,
                        stdc::formatTextN(R"(%1: "configuration" field has invalid value)", configPath),
                    };
                    return false;
                }
                configuration_ = it->second.toObject();
            }
        }

        path = fs::canonical(configPath).parent_path();
        id = std::move(id_);
        model = std::move(model_);
        name = std::move(name_);
        avatar = std::move(avatar_);
        background = std::move(background_);
        demoAudio = std::move(demoAudio_);
        imports = std::move(imports_);
        configuration = std::move(configuration_);
        return true;
    }

    SingerSpec::~SingerSpec() = default;

    std::filesystem::path SingerSpec::path() const {
        __stdc_impl_t;
        return impl.path;
    }

    std::string SingerSpec::model() const {
        __stdc_impl_t;
        return impl.model;
    }

    DisplayText SingerSpec::name() const {
        __stdc_impl_t;
        return impl.name;
    }

    std::filesystem::path SingerSpec::avatar() const {
        __stdc_impl_t;
        return impl.avatar;
    }

    std::filesystem::path SingerSpec::background() const {
        __stdc_impl_t;
        return impl.background;
    }

    std::filesystem::path SingerSpec::demoAudio() const {
        __stdc_impl_t;
        return impl.demoAudio;
    }

    const std::vector<SingerImport> &SingerSpec::imports() const {
        __stdc_impl_t;
        return impl.imports;
    }

    JsonObject SingerSpec::configuration() const {
        __stdc_impl_t;
        return impl.configuration;
    }

    std::vector<Inference *> SingerSpec::createInferences(Error *error) {
        __stdc_impl_t;
        return {};
    }

    SingerSpec::SingerSpec() : ContributeSpec(*new Impl()) {
    }

}