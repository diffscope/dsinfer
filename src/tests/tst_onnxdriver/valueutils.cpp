#include "valueutils.h"

#include <fstream>
#include <sstream>
#include <type_traits>
#include <vector>

namespace fs = std::filesystem;
namespace DS = dsinfer;

namespace ValueUtils {

    // Read JsonValue from file path
    DS::JsonValue jsonValueFromPath(const std::filesystem::path &path) {
        std::ifstream jsonFile(path);
        if (!jsonFile.is_open()) {
            return {DS::JsonValue::Undefined};
        }
        std::string jsonString = (std::stringstream() << jsonFile.rdbuf()).str();
        return DS::JsonValue::fromJson(jsonString, false);
    }

    // Stringify task input/output JsonValue detail
    std::string inferValueStringify(const dsinfer::JsonValue &value) {
        const auto format = value["format"].toString();
        const auto &data = value["data"];
        std::ostringstream oss;
        oss << '(';
        if (auto name = value["name"].toString(); !name.empty()) {
            oss << "name=\"" << name << "\"";
            if (auto class_ = value["class"].toString(); !class_.empty()) {
                oss << ", class=\"" << class_ << "\"";
            }
        } else if (auto class_ = value["class"].toString(); !class_.empty()) {
            oss << "class=\"" << class_ << "\"";
        }
        oss << ", format=" << format;
        if (format == "bytes") {
            auto type = data["type"].toString();
            oss << ", type=" << type << ") ";
            auto outArr = data["value"].toBinary();
            if (type == "float" || type == "float32") {
                oss << arrayStringify(reinterpret_cast<float *>(outArr.data()),
                                      outArr.size() / sizeof(float));
            } else if (type == "int64") {
                oss << arrayStringify(reinterpret_cast<int64_t *>(outArr.data()),
                                      outArr.size() / sizeof(int64_t));
            } else if (type == "bool") {
                oss << arrayStringify(reinterpret_cast<bool *>(outArr.data()),
                                      outArr.size() / sizeof(bool));
            } else {
                oss << "Unsupported type: " << type;
            }
        } else if (format == "array") {
            auto type = data["type"].toString();
            oss << ", type=" << type << ") ";
            auto outArr = data["value"].toArray();
            oss << arrayStringify(outArr.data(), outArr.size());
        } else if (format == "reference") {
            oss << ", key=\"" << data["value"].toString() << "\")";
        }
        return oss.str();
    }
}