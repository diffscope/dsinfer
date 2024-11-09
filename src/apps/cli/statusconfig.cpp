#include "statusconfig.h"

#include <fstream>
#include <sstream>

#include <stdcorelib/strings.h>

#include <dsinfer/jsonvalue.h>
#include <dsinfer/contributespec.h>

using namespace dsinfer;

namespace cli {

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
        auto root = JsonValue::fromJson(ss.str(), true, &error);
        if (!error.empty() || !root.isObject()) {
            return false;
        }
        auto obj = root.toObject();

        // packages
        std::vector<Package> packages_;
        do {
            auto it = obj.find("packages");
            if (it == obj.end() || !it->second.isArray()) {
                break;
            }
            for (const auto &item : it->second.toArray()) {
                if (!item.isObject()) {
                    continue;
                }
                auto packageObj = item.toObject();
                Package pkg;

                // id[version]
                {
                    auto it2 = packageObj.find("id");
                    if (it2 == packageObj.end() || !it2->second.isString()) {
                        continue;
                    }
                    std::string id = it2->second.toString();
                    if (id.empty()) {
                        continue;
                    }
                    auto identifier = ContributeIdentifier::fromString(id);
                    if (!identifier.library().empty() && !identifier.version().isEmpty() &&
                        identifier.id().empty()) {
                        pkg.id = identifier.library();
                        pkg.version = identifier.version();
                    } else {
                        continue;
                    }
                }
                // relativeLocation
                {
                    auto it2 = packageObj.find("relativeLocation");
                    if (it2 == packageObj.end() || !it2->second.isString()) {
                        continue;
                    }
                    std::string path_ = it2->second.toString();
                    if (path_.empty()) {
                        continue;
                    }
                    pkg.relativeLocation = path_;
                }
                // metadata
                {
                    auto it2 = packageObj.find("metadata");
                    if (it2 == packageObj.end() || !it2->second.isObject()) {
                        continue;
                    }
                    const auto &metadataObj = it2->second.toObject();
                    Package::Metadata metadata_;

                    // hasSinger (optional)
                    do {
                        auto it3 = metadataObj.find("hasSinger");
                        if (it3 == packageObj.end() || !it3->second.isBool()) {
                            break;
                        }
                        metadata_.hasSinger = it3->second.toBool();
                    } while (false);

                    // installedTimestamp (optional)
                    do {
                        auto it3 = metadataObj.find("installedTimestamp");
                        if (it3 == packageObj.end() || !it3->second.isInt()) {
                            break;
                        }
                        metadata_.installedTimestamp = it3->second.toInt64();
                    } while (false);

                    pkg.metadata = std::move(metadata_);
                }
                packages_.emplace_back(pkg);
            }
        } while (false);

        packages = std::move(packages_);
        return true;
    }

    bool StatusConfig::save(const std::filesystem::path &path) const {
        JsonObject obj;

        // packages
        {
            JsonArray packagesArr;
            for (const auto &packageItem : std::as_const(packages)) {
                JsonObject pkgObj;

                // id
                pkgObj["id"] =
                    stdc::formatN("%1[%2]", packageItem.id, packageItem.version.toString());

                // relativeLocation
                pkgObj["relativeLocation"] = packageItem.relativeLocation;

                // metadata
                {
                    JsonObject metadataObj;
                    const auto &metadata = packageItem.metadata;

                    // contributes
                    metadataObj["hasSinger"] = metadata.hasSinger;

                    // installedTimestamp
                    if (metadata.installedTimestamp != 0) {
                        metadataObj["installedTimestamp"] = metadata.installedTimestamp;
                    }

                    pkgObj["metadata"] = std::move(metadataObj);
                }

                packagesArr.emplace_back(pkgObj);
            }
            obj["packages"] = std::move(packagesArr);
        }

        std::ofstream file(path);
        if (!file.is_open()) {
            return false;
        }

        auto data = JsonValue(obj).toString();
        file.write(data.data(), std::streamsize(data.size()));
        return false;
    }

}