#ifndef DSINFER_CLI_STATUSCONFIG_H
#define DSINFER_CLI_STATUSCONFIG_H

#include <vector>
#include <filesystem>

#include <dsinfer/versionnumber.h>

namespace cli {

    class StatusConfig {
    public:
        StatusConfig();
        ~StatusConfig();

    public:
        bool load(const std::filesystem::path &path);
        bool save(const std::filesystem::path &path) const;

    public:
        struct Package {
            // identifier
            std::string id;
            dsinfer::VersionNumber version;

            // basic
            std::string relativeLocation;

            // metadata
            struct Metadata {
                bool hasSinger = false;
                int64_t installedTimestamp = 0;
            };
            Metadata metadata;
        };
        std::vector<Package> packages;
    };

}

#endif // DSINFER_CLI_STATUSCONFIG_H
