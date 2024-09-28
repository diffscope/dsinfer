#include "singerspec.h"
#include "singerspec_p.h"

#include <fstream>

#include "format.h"
#include "inferencespec.h"

namespace fs = std::filesystem;

namespace dsinfer {

    static SingerImport readSingerImport(const JsonValue &val) {
        if (val.isString()) {
            auto inference = ContributeIdentifier::fromString(val.toString());
            SingerImport res;
            res.inference = inference;
            return res;
        }

        auto obj = val.toObject();
        auto it = obj.find("id");
        if (it == obj.end()) {
            return {};
        }
        auto inference = ContributeIdentifier::fromString(val.toString());
        SingerImport res;
        res.inference = inference;
        it = obj.find("options");
        if (it != obj.end()) {
            res.options = it->second;
        }
        return res;
    }

    bool SingerSpec::Impl::read(const std::filesystem::path &basePath, const JsonObject &obj,
                                Error *error) {
        fs::path configPath;
        std::string id_;
        std::string model_;

        std::string name_;

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
            if (id_.empty()) {
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

            configPath = pathFromString(configPathString);
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
                    formatTextN(R"(failed to open singer manifest "%1")", configPath),
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
                    formatTextN(R"(invalid singer manifest format "%1": %2)", configPath, error2),
                };
                return false;
            }
            if (!root.isObject()) {
                *error = {
                    Error::InvalidFormat,
                    formatTextN(R"(invalid singer manifest format "%1")", configPath),
                };
                return false;
            }
            configObj = root.toObject();
        }

        // Get attributes
        // name
        {
            auto it = configObj.find("name");
            if (it == configObj.end()) {
                *error = {
                    Error::InvalidFormat,
                    R"(missing "name" field in singer manifest)",
                };
                return false;
            }
            name_ = it->second.toString();
            if (name_.empty()) {
                *error = {
                    Error::InvalidFormat,
                    R"("name" field has invalid value in singer manifest)",
                };
                return false;
            }
        }
        // avatar
        {
            auto it = configObj.find("avatar");
            if (it != configObj.end()) {
                avatar_ = pathFromString(it->second.toString());
            }
        }
        // background
        {
            auto it = configObj.find("background");
            if (it != configObj.end()) {
                background_ = pathFromString(it->second.toString());
            }
        }
        // demoAudio
        {
            auto it = configObj.find("demoAudio");
            if (it != configObj.end()) {
                demoAudio_ = pathFromString(it->second.toString());
            }
        }
        // imports
        {
            auto it = configObj.find("imports");
            if (it != configObj.end()) {
                if (!it->second.isArray()) {
                    *error = {
                        Error::InvalidFormat,
                        R"("imports" field has invalid value in singer manifest)",
                    };
                    return false;
                }

                for (const auto &item : it->second.toArray()) {
                    auto singerImport = readSingerImport(item);
                    if (singerImport.inference.id().empty()) {
                        *error = {
                            Error::InvalidFormat,
                            R"(unknown data in "imports" field in singer manifest)",
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
                        R"("configuration" field has invalid value in singer manifest)",
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
        return false;
    }

    SingerSpec::~SingerSpec() = default;

    std::filesystem::path SingerSpec::path() const {
        __dsinfer_impl_t;
        return impl.path;
    }

    std::string SingerSpec::model() const {
        __dsinfer_impl_t;
        return impl.model;
    }

    std::string SingerSpec::name() const {
        __dsinfer_impl_t;
        return impl.name;
    }

    std::filesystem::path SingerSpec::avatar() const {
        __dsinfer_impl_t;
        return impl.avatar;
    }

    std::filesystem::path SingerSpec::background() const {
        __dsinfer_impl_t;
        return impl.background;
    }

    std::filesystem::path SingerSpec::demoAudio() const {
        __dsinfer_impl_t;
        return impl.demoAudio;
    }

    const std::vector<SingerImport> &SingerSpec::imports() const {
        __dsinfer_impl_t;
        return impl.imports;
    }

    JsonObject SingerSpec::configuration() const {
        __dsinfer_impl_t;
        return impl.configuration;
    }

    std::vector<Inference *> SingerSpec::createInferences(Error *error) const {
        __dsinfer_impl_t;
        return {};
    }

    SingerSpec::SingerSpec() : ContributeSpec(*new Impl()) {
    }

}