#include "startupconfig.h"

#include <fstream>
#include <sstream>

#include <dsinfer/jsonvalue.h>
#include <dsinfer/format.h>

using namespace dsinfer;

namespace cli {

    StartupConfig::StartupConfig() = default;

    StartupConfig::~StartupConfig() = default;

    bool StartupConfig::load(const std::filesystem::path &path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            return false;
        }

        std::stringstream ss;
        ss << file.rdbuf();

        std::string error;
        auto root = JsonValue::fromJson(ss.str(), true, &error);
        if (!error.empty() || !root.isObject()) {
            return false;
        }
        auto obj = root.toObject();

        // paths
        std::vector<std::filesystem::path> paths_;
        {
            auto it = obj.find("paths");
            if (it == obj.end() || !it->second.isArray()) {
                return false;
            }
            for (const auto &pathItem : it->second.toArray()) {
                if (!pathItem.isString()) {
                    return false;
                }
                auto pathStr = pathItem.toString();
                if (pathStr.empty()) {
                    return false;
                }
                paths_.emplace_back(pathFromString(pathStr));
            }
        }

        // driver
        Driver driver_;
        do {
            auto it = obj.find("driver");
            if (it == obj.end() || !it->second.isObject()) {
                break;
            }
            auto driverObj = it->second.toObject();

            // id
            do {
                auto it2 = driverObj.find("id");
                if (it2 == driverObj.end() || !it2->second.isString()) {
                    break;
                }
                driver_.id = std::move(it2->second.toString());
            } while (false);
            if (driver_.id.empty()) {
                break;
            }

            // init
            do {
                auto it2 = driverObj.find("init");
                if (it2 == driverObj.end() || !it2->second.isString()) {
                    break;
                }
                driver_.init = it2->second;
            } while (false);
        } while (false);
        paths = std::move(paths_);
        driver = std::move(driver_);
        return true;
    }

}