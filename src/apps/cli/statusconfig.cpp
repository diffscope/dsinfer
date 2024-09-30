#include "statusconfig.h"

#include <fstream>
#include <sstream>

#include <dsinfer/jsonvalue.h>
#include <dsinfer/contributespec.h>
#include <dsinfer/format.h>

using namespace dsinfer;

StatusConfig::~StatusConfig() = default;

StatusConfig::StatusConfig() = default;

bool StatusConfig::load(const std::filesystem::path &path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return false;
    }

    std::stringstream ss;
    ss << file.rdbuf();

    std::string error;
    auto root = JsonValue::fromJson(ss.str(), &error);
    if (!error.empty() || !root.isObject()) {
        return false;
    }
    auto obj = root.toObject();

    // packages
    std::vector<Package> packages_;
    {
        auto it = obj.find("packages");
        if (it == obj.end() || !it->second.isArray()) {
            return false;
        }
        for (const auto &item : it->second.toArray()) {
            if (!item.isObject()) {
                return false;
            }
            auto packageObj = item.toObject();

            Package pkg;
            // id[version]
            {
                auto it2 = packageObj.find("id");
                if (it2 == packageObj.end() || !it2->second.isString()) {
                    return false;
                }
                std::string id = it2->second.toString();
                if (id.empty()) {
                    return false;
                }
                auto identifier = ContributeIdentifier::fromString(id);
                if (!identifier.library().empty() && !identifier.version().isEmpty() &&
                    identifier.id().empty()) {
                    pkg.id = identifier.library();
                    pkg.version = identifier.version();
                } else {
                    return false;
                }
            }
            // path
            {
                auto it2 = packageObj.find("path");
                if (it2 == packageObj.end() || !it2->second.isString()) {
                    return false;
                }
                std::string path_ = it2->second.toString();
                if (path_.empty()) {
                    return false;
                }
                pkg.path = path_;
            }
            // contributes
            {
                auto it2 = packageObj.find("contributes");
                if (it2 == packageObj.end() || !it2->second.isArray()) {
                    return false;
                }
                for (const auto &contributeItem : it2->second.toArray()) {
                    if (!contributeItem.isString()) {
                        return false;
                    }
                    auto contribute = contributeItem.toString();
                    if (contribute.empty()) {
                        return false;
                    }
                    pkg.contributes.emplace_back(contribute);
                }
            }
            packages_.emplace_back(pkg);
        }
    }

    packages = std::move(packages_);
    return true;
}

bool StatusConfig::save(const std::filesystem::path &path) const {
    JsonObject obj;

    // packages
    {
        JsonArray packagesArr;
        for (const auto &packageItem : std::as_const(packages)) {
            JsonObject packageObj;
            packageObj["id"] =
                formatTextN("%1[%2]", packageItem.id, packageItem.version.toString());

            JsonArray contributesArr;
            for (const auto &contributeItem : std::as_const(packageItem.contributes)) {
                contributesArr.emplace_back(contributeItem);
            }
            packageObj["contributes"] = contributesArr;

            packagesArr.emplace_back(packageObj);
        }
        obj["packages"] = packagesArr;
    }

    std::ofstream file(path);
    if (!file.is_open()) {
        return false;
    }

    auto data = JsonValue(obj).toString();
    file.write(data.data(), std::streamsize(data.size()));
    return false;
}
