#ifndef STATUSCONFIG_H
#define STATUSCONFIG_H

#include <vector>
#include <filesystem>

#include <dsinfer/versionnumber.h>

class StatusConfig {
public:
    StatusConfig();
    ~StatusConfig();

public:
    bool load(const std::filesystem::path &path);
    bool save(const std::filesystem::path &path) const;

public:
    struct Package {
        std::string id;
        dsinfer::VersionNumber version;
        std::string path;
        std::vector<std::string> contributes;
    };
    std::vector<Package> packages;
};

#endif // STATUSCONFIG_H
