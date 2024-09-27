#include "inferencespec.h"
#include "contributespec_p.h"

#include "format.h"
#include "libraryspec.h"
#include "inferenceinterpreter.h"

#include <fstream>
#include <sstream>

namespace fs = std::filesystem;

namespace dsinfer {

    class InferenceSpec::Impl : public ContributeSpec::Impl {
    public:
        Impl() : ContributeSpec::Impl(ContributeSpec::Inference) {
        }

        bool read(const std::filesystem::path &basePath, const dsinfer::JsonObject &obj,
                  Error *error) override {
            fs::path configPath;
            std::string id_;
            std::string className_;

            std::string name_;
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
                        R"(missing "id" field in brief manifest)",
                    };
                    return false;
                }
                id_ = it->second.toString();
                if (id_.empty()) {
                    *error = {
                        Error::InvalidFormat,
                        R"("id" field has invalid value in brief manifest)",
                    };
                    return false;
                }

                // class
                it = obj.find("class");
                if (it == obj.end()) {
                    *error = {
                        Error::InvalidFormat,
                        R"(missing "class" field in brief manifest)",
                    };
                    return false;
                }
                className_ = it->second.toString();
                if (className_.empty()) {
                    *error = {
                        Error::InvalidFormat,
                        R"("class" field has invalid value in brief manifest)",
                    };
                    return false;
                }

                // configuration
                it = obj.find("configuration_");
                if (it == obj.end()) {
                    *error = {
                        Error::InvalidFormat,
                        R"(missing "configuration" field in brief manifest)",
                    };
                    return false;
                }

                std::string configPathString = it->second.toString();
                if (configPathString.empty()) {
                    *error = {
                        Error::InvalidFormat,
                        R"("configuration" field has invalid value in brief manifest)",
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
                        formatTextN(R"(failed to open configuration "%1")", configPath),
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
                        formatTextN(R"(invalid configuration format "%1": %2)", configPath, error2),
                    };
                    return false;
                }
                if (!root.isObject()) {
                    *error = {
                        Error::InvalidFormat,
                        formatTextN(R"(invalid configuration format "%1")", configPath),
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
                        R"(missing "name" field in configuration manifest)",
                    };
                    return false;
                }
                name_ = it->second.toString();
                if (name_.empty()) {
                    *error = {
                        Error::InvalidFormat,
                        R"("name" field has invalid value in configuration manifest)",
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
                        R"(missing "level" field in configuration manifest)",
                    };
                    return false;
                }
                apiLevel_ = it->second.toInt();
                if (apiLevel_ == 0) {
                    *error = {
                        Error::InvalidFormat,
                        R"("level" field has invalid value in configuration manifest)",
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
                            R"("schema" field has invalid value in configuration manifest)",
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
                            R"("configuration" field has invalid value in configuration manifest)",
                        };
                        return false;
                    }
                    configuration_ = it->second.toObject();
                }
            }

            path = fs::canonical(configPath).parent_path();
            id = id_;
            className = className_;
            name = name_;
            apiLevel = apiLevel_;
            schema = schema_;
            configuration = configuration_;
            return true;
        }

        std::filesystem::path path;

        std::string id;
        std::string className;

        std::string name;
        int apiLevel = 0;

        JsonObject schema;
        JsonObject configuration;
    };

    InferenceSpec::~InferenceSpec() = default;

    std::filesystem::path InferenceSpec::path() const {
        __dsinfer_impl_t;
        return impl.path;
    }

    std::string InferenceSpec::className() const {
        __dsinfer_impl_t;
        return impl.className;
    }

    std::string InferenceSpec::name() const {
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

        auto interp = env()->plugin<InferenceInterpreter>(impl.className.data());
        if (!interp) {
            *error = {
                Error::FeatureNotSupported,
                formatTextN("Interpreter \"%1\" not found", impl.className),
            };
            return nullptr;
        }
        return interp->create(this, options, error);
    }

    InferenceSpec::InferenceSpec() : ContributeSpec(*new Impl()) {
    }

}