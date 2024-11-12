#include "startupconfig.h"

#include <fstream>
#include <sstream>

#include <stdcorelib/path.h>
#include <stdcorelib/console.h>

#include <dsinfer/jsonvalue.h>

#include "utils.h"

using namespace dsinfer;

namespace fs = std::filesystem;

namespace cli {

    static const std::map<std::string, std::string> &systemPaths() {
        static auto instance = []() -> std::map<std::string, std::string> {
            return {
                {"HOME", stdc::path::to_utf8(home_dir())}
            };
        }();
        return instance;
    }

    StartupConfig::StartupConfig() = default;

    StartupConfig::~StartupConfig() = default;

    bool StartupConfig::load(const fs::path &path) {
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
        std::vector<fs::path> paths_;
        do {
            auto it = obj.find("paths");
            if (it == obj.end() || !it->second.isArray()) {
                break;
            }
            for (const auto &pathItem : it->second.toArray()) {
                if (!pathItem.isString()) {
                    continue;
                }
                auto pathStr = pathItem.toString();
                if (pathStr.empty()) {
                    continue;
                }
                // Replace variables
                pathStr = stdc::strings::parse_expr(pathStr, systemPaths());
                auto path_ = stdc::path::from_utf8(pathStr);
                // To absolute
                if (path_.is_relative()) {
                    path_ = path.parent_path() / path_;
                }
                // To canonical
                path_ = stdc::path::canonical(path_);
                if (path_.empty()) {
                    continue;
                }
                paths_.emplace_back(path_);
            }
        } while (false);

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