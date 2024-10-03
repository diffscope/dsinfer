#ifndef STARTUPCONFIG_H
#define STARTUPCONFIG_H

#include <filesystem>

#include <dsinfer/jsonvalue.h>

namespace cli {

    class StartupConfig {
    public:
        StartupConfig();
        ~StartupConfig();

    public:
        bool load(const std::filesystem::path &path);

    public:
        struct Driver {
            std::string id;
            dsinfer::JsonValue init;
        };

        std::vector<std::filesystem::path> paths;
        Driver driver;
    };

}

#endif // STARTUPCONFIG_H
