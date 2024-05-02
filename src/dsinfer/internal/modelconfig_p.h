#ifndef MODELCONFIG_P_H
#define MODELCONFIG_P_H

#include <string>
#include <filesystem>
#include <vector>

#include <utils/simplevarexp.h>
#include <dsinfer/dsinfer_common.h>

namespace dsinfer {

    struct ModelDependency {
        std::string id;
        std::string version;
        std::string level;
    };

    class ModelConfig {
    public:
        virtual ~ModelConfig();

        void load(const std::filesystem::path &path, const SimpleVarExp &varMap);

    public:
        std::string id;
        std::string version;
        int level = 0;

        ModelType type;
        std::filesystem::path path;

        std::vector<ModelDependency> dependencies;

    protected:
        virtual void loadArguments(const SimpleVarExp &varMap);
    };

}

#endif // MODELCONFIG_P_H
