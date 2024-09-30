#ifndef LOADCONFIG_H
#define LOADCONFIG_H

#include <filesystem>

#include <dsinfer/jsonvalue.h>

class LoadConfig {
public:
    LoadConfig();
    ~LoadConfig();

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

#endif // LOADCONFIG_H
