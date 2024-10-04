#include "inferencespec.h"
#include "inferencespec_p.h"

#include "format.h"
#include "libraryspec.h"

#include <fstream>
#include <sstream>

namespace fs = std::filesystem;

namespace dsinfer {

    bool InferenceSpec::Impl::read(const std::filesystem::path &basePath, const JsonObject &obj,
                                   Error *error) {
        fs::path configPath;
        VersionNumber fmtVersion_;
        std::string id_;
        std::string className_;

        DisplayText name_;
        int apiLevel_;

        JsonObject schema_;
        JsonObject configuration_;

        // Parse desc
        {
            // id
            auto it = obj.find("id");
            if (it == obj.end()) {
                *error = {
                    Error::InvalidFormat,
                    R"(missing "id" field in inference contribute field)",
                };
                return false;
            }
            id_ = it->second.toString();
            if (!ContributeIdentifier::isValidId(id_)) {
                *error = {
                    Error::InvalidFormat,
                    R"("id" field has invalid value in inference contribute field)",
                };
                return false;
            }

            // class
            it = obj.find("class");
            if (it == obj.end()) {
                *error = {
                    Error::InvalidFormat,
                    R"(missing "class" field in inference contribute field)",
                };
                return false;
            }
            className_ = it->second.toString();
            if (className_.empty()) {
                *error = {
                    Error::InvalidFormat,
                    R"("class" field has invalid value in inference contribute field)",
                };
                return false;
            }

            // configuration
            it = obj.find("configuration");
            if (it == obj.end()) {
                *error = {
                    Error::InvalidFormat,
                    R"(missing "configuration" field in inference contribute field)",
                };
                return false;
            }

            std::string configPathString = it->second.toString();
            if (configPathString.empty()) {
                *error = {
                    Error::InvalidFormat,
                    R"("configuration" field has invalid value in inference contribute field)",
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
                    formatTextN(R"(%1: failed to open inference manifest)", configPath),
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
                    formatTextN(R"(%1: invalid inference manifest format: %2)", configPath, error2),
                };
                return false;
            }
            if (!root.isObject()) {
                *error = {
                    Error::InvalidFormat,
                    formatTextN(R"(%1: invalid inference manifest format)", configPath),
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
                    formatTextN(R"(%1: missing "$version" field)", configPath),
                };
                return false;
            }
            fmtVersion_ = VersionNumber::fromString(it->second.toString());
            if (fmtVersion_ > VersionNumber(1)) {
                *error = {
                    Error::FeatureNotSupported,
                    formatTextN(R"(%1: format version "%2" is not supported)", configPath,
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
        // level
        {
            auto it = configObj.find("level");
            if (it == configObj.end()) {
                *error = {
                    Error::InvalidFormat,
                    formatTextN(R"(%1: missing "level" field)", configPath),
                };
                return false;
            }
            apiLevel_ = it->second.toInt();
            if (apiLevel_ == 0) {
                *error = {
                    Error::InvalidFormat,
                    formatTextN(R"(%1: "level" field has invalid value)", configPath),
                };
                return false;
            }
        }
        // schema
        {
            auto it = configObj.find("schema");
            if (it != configObj.end()) {
                if (!it->second.isObject()) {
                    *error = {
                        Error::InvalidFormat,
                        formatTextN(R"(%1: "schema" field has invalid value)", configPath),
                    };
                    return false;
                }
                schema_ = it->second.toObject();
            }
        }
        // configuration
        {
            auto it = configObj.find("configuration");
            if (it != configObj.end()) {
                if (!it->second.isObject()) {
                    *error = {
                        Error::InvalidFormat,
                        formatTextN(R"(%1: "configuration" field has invalid value)", configPath),
                    };
                    return false;
                }
                configuration_ = it->second.toObject();
            }
        }

        path = fs::canonical(configPath).parent_path();
        fmtVersion = fmtVersion_;
        id = std::move(id_);
        className = std::move(className_);
        name = std::move(name_);
        apiLevel = apiLevel_;
        schema = std::move(schema_);
        configuration = std::move(configuration_);
        return true;
    }

    InferenceSpec::~InferenceSpec() = default;

    std::filesystem::path InferenceSpec::path() const {
        __dsinfer_impl_t;
        return impl.path;
    }

    std::string InferenceSpec::className() const {
        __dsinfer_impl_t;
        return impl.className;
    }

    DisplayText InferenceSpec::name() const {
        __dsinfer_impl_t;
        return impl.name;
    }

    int InferenceSpec::apiLevel() const {
        __dsinfer_impl_t;
        return impl.apiLevel;
    }

    JsonObject InferenceSpec::schema() const {
        __dsinfer_impl_t;
        return impl.schema;
    }

    JsonObject InferenceSpec::configuration() const {
        __dsinfer_impl_t;
        return impl.configuration;
    }

    Inference *InferenceSpec::create(const JsonObject &options, Error *error) const {
        __dsinfer_impl_t;
        return impl.interp->create(this, options, error);
    }

    bool InferenceSpec::validate(const JsonValue &options, std::string *error) const {
        __dsinfer_impl_t;
        return impl.interp->validate(this, options, error);
    }

    InferenceSpec::InferenceSpec() : ContributeSpec(*new Impl()) {
    }

}