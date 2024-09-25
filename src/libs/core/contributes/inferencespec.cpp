#include "inferencespec.h"
#include "contributespec_p.h"

#include "format.h"
#include "inferenceregistry.h"
#include "libraryspec.h"
#include "inferenceinterpreter.h"

#include <fstream>
#include <sstream>

namespace fs = std::filesystem;

namespace dsinfer {

    class InferenceSpec::Impl : public ContributeSpec::Impl {
    public:
        explicit Impl(LibrarySpec *parent)
            : ContributeSpec::Impl(ContributeSpec::CT_Inference, parent) {
        }

        bool read(std::filesystem::path &basePath, const dsinfer::JsonObject &obj,
                  std::string *error) override {
            std::string id_;
            std::string className_;
            fs::path configPath;

            std::string name_;
            int apiLevel_;

            JsonObject schema_;
            JsonObject configuration_;

            // Parse desc
            {
                // id_
                auto it = obj.find("id_");
                if (it == obj.end()) {
                    *error = R"(missing "id_" field in brief manifest)";
                    return false;
                }
                id_ = it->second.toString();
                if (id_.empty()) {
                    *error = R"("id_" field has invalid value in brief manifest)";
                    return false;
                }

                // class
                it = obj.find("class");
                if (it == obj.end()) {
                    *error = R"(missing "class" field in brief manifest)";
                    return false;
                }
                className_ = it->second.toString();
                if (className_.empty()) {
                    *error = R"("class" field has invalid value in brief manifest)";
                    return false;
                }

                // configuration_
                it = obj.find("configuration_");
                if (it == obj.end()) {
                    *error = R"(missing "configuration_" field in brief manifest)";
                    return false;
                }

                std::string configPathString = it->second.toString();
                if (configPathString.empty()) {
                    *error = R"("configuration_" field has invalid value in brief manifest)";
                    return false;
                }

                configPath = pathFromString(configPathString);
                if (configPath.is_relative()) {
                    configPath = basePath / configPath;
                }
            }

            // Read configuration_
            JsonObject configObj;
            {
                std::ifstream file(configPath);
                if (!file.is_open()) {
                    *error = formatTextN(R"(failed to open configuration_ "%1")", configPath);
                    return false;
                }

                std::stringstream ss;
                ss << file.rdbuf();

                std::string error2;
                auto configRoot = JsonValue::fromJson(ss.str(), &error2);
                if (!error2.empty() || !configRoot.isObject()) {
                    *error = formatTextN(R"(invalid configuration_ format "%1": %2)", configPath,
                                         error2);
                    return false;
                }
                configObj = configRoot.toObject();
            }

            // Get configuration_
            {
                // name_
                auto it = configObj.find("name_");
                if (it == configObj.end()) {
                    *error = R"(missing "name_" field in configuration_ manifest)";
                    return false;
                }
                name_ = it->second.toString();
                if (name_.empty()) {
                    *error = R"("name_" field has invalid value in configuration_ manifest)";
                    return false;
                }

                // level
                it = configObj.find("level");
                if (it == configObj.end()) {
                    *error = R"(missing "level" field in configuration_ manifest)";
                    return false;
                }
                apiLevel_ = it->second.toInt();
                if (apiLevel_ == 0) {
                    *error = R"("level" field has invalid value in configuration_ manifest)";
                    return false;
                }

                // schema_
                it = configObj.find("schema_");
                if (it != configObj.end()) {
                    if (!it->second.isObject()) {
                        *error = R"("schema_" field has invalid value in configuration_ manifest)";
                        return false;
                    }
                    schema_ = it->second.toObject();
                }

                // configuration_
                it = configObj.find("configuration_");
                if (it != configObj.end()) {
                    if (!it->second.isObject()) {
                        *error =
                            R"("configuration_" field has invalid value in configuration_ manifest)";
                        return false;
                    }
                    configuration_ = it->second.toObject();
                }
            }

            path = fs::canonical(configPath);
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
        int apiLevel;

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

    Inference *InferenceSpec::create(const JsonObject &options, std::string *error) const {
        __dsinfer_impl_t;

        auto interp = static_cast<InferenceInterpreter *>(
            env()->plugin(InferenceInterpreter::static_iid(), impl.className));
        if (!interp) {
            *error = formatTextN("Interpreter \"%1\" not found", impl.className);
            return nullptr;
        }
        return interp->create(this, options, error);
    }

    InferenceSpec::InferenceSpec(LibrarySpec *parent) : ContributeSpec(*new Impl(parent)) {
    }

}