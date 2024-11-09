#include "valueutils.h"

#include <fstream>
#include <sstream>
#include <type_traits>
#include <vector>

namespace fs = std::filesystem;
namespace DS = dsinfer;

// Template to check if a type is any string type
template<typename T>
struct is_any_string {
    static const bool value = std::is_same<T, std::string>::value ||
                              std::is_same<T, std::wstring>::value ||
                              std::is_same<T, const char*>::value ||
                              std::is_same<T, const wchar_t*>::value;
};

template <typename T>
std::string arrayStringify(const T *buf, size_t size) {
    std::ostringstream oss;
    oss << '[';
    for (size_t i = 0; i < size; ++i) {
        if (i > 0) {
            oss << ", ";
        }
        if constexpr (std::is_same_v<T, bool>) {
            oss << (buf[i] ? "true" : "false");
        } else if constexpr (is_any_string<T>::value) {
            oss << "\"" << buf[i] << "\"";
        } else if constexpr (std::is_same_v<T, DS::JsonValue>) {
            oss << buf[i]->toJson();
        } else {
            oss << buf[i];
        }
    }
    oss << ']';
    return oss.str();
}

template <typename T>
std::string arrayStringify(const std::vector<T> &arr) {
    return arrayStringify(arr.data(), arr.size());
}

template <>
std::string arrayStringify(const std::vector<bool> &arr) {
    std::ostringstream oss;
    oss << '[';
    bool isFirstFlag = true;
    for (const auto &item: arr) {
        if (isFirstFlag) {
            isFirstFlag = false;
        } else {
            oss << ", ";
        }
        oss << (item ? "true" : "false");
    }
    oss << ']';
    return oss.str();
}

ValueUtils::ValueUtils(dsinfer::Log::Category *logger_) : logger(logger_) {
}

ValueUtils::~ValueUtils() = default;

// Read JsonValue from file path
dsinfer::JsonValue ValueUtils::jsonValueFromPath(const std::filesystem::path &path) {
    logger->info("Reading json file from %1", path);
    std::ifstream jsonFile(path);
    if (!jsonFile.is_open()) {
        logger->critical("Failed to read json file!");
        return { DS::JsonValue::Undefined };
    }
    std::string jsonString = (std::stringstream() << jsonFile.rdbuf()).str();
    return DS::JsonValue::fromJson(jsonString, false);
}

// Stringify task input/output JsonValue detail
std::string ValueUtils::inferValueStringify(const dsinfer::JsonValue &value) {
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
        oss << data["value"].toJson();
    } else if (format == "reference") {
        oss << ", key=\"" << data["value"].toString() << "\")";
    }
    return oss.str();
}
