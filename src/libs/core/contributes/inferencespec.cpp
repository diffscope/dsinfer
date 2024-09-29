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
            if (id_.empty()) {
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
                    formatTextN(R"(failed to open inference manifest "%1")", configPath),
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
                    formatTextN(R"(invalid inference manifest format "%1": %2)", configPath,
                                error2),
                };
                return false;
            }
            if (!root.isObject()) {
                *error = {
                    Error::InvalidFormat,
                    formatTextN(R"(invalid inference manifest format "%1")", configPath),
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
                    R"(missing "name" field in inference manifest)",
                };
                return false;
            }
            name_ = it->second;
            if (name_.isEmpty()) {
                *error = {
                    Error::InvalidFormat,
                    R"("name" field has invalid value in inference manifest)",
                };
                return false;
            }
        }
        // level
        {
            auto it = configObj.find("level");
            if (it == configObj.end()) {
                *error = {
                    Error::InvalidFormat,
                    R"(missing "level" field in inference manifest)",
                };
                return false;
            }
            apiLevel_ = it->second.toInt();
            if (apiLevel_ == 0) {
                *error = {
                    Error::InvalidFormat,
                    R"("level" field has invalid value in inference manifest)",
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
                        R"("schema" field has invalid value in inference manifest)",
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
                        R"("configuration" field has invalid value in inference manifest)",
                    };
                    return false;
                }
                configuration_ = it->second.toObject();
            }
        }

        path = fs::canonical(configPath).parent_path();
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