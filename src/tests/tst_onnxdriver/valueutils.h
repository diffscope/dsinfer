#ifndef TST_ONNXDRIVER_VALUEUTILS_H
#define TST_ONNXDRIVER_VALUEUTILS_H

#include <filesystem>

#include <dsinfer/log.h>
#include <dsinfer/jsonvalue.h>

namespace ValueUtils {

    dsinfer::JsonValue jsonValueFromPath(const std::filesystem::path &path);
    std::string inferValueStringify(const dsinfer::JsonValue &value);

    // Template to check if a type is any string type
    template <typename T>
    struct is_any_string {
        static const bool value =
            std::is_same<T, std::string>::value || std::is_same<T, std::wstring>::value ||
            std::is_same<T, const char *>::value || std::is_same<T, const wchar_t *>::value;
    };

    template <typename T>
    inline std::string formatElement(const T &element) {
        if constexpr (std::is_same_v<T, bool>) {
            return element ? "true" : "false";
        } else if constexpr (is_any_string<T>::value) {
            return "\"" + std::string(element) + "\"";
        } else if constexpr (std::is_same_v<T, dsinfer::JsonValue>) {
            return element.toJson();
        } else {
            return std::to_string(element);
        }
    }

    template <typename T>
    inline std::string arrayStringify(const T *buf, size_t size) {
        std::ostringstream oss;
        oss << '[';
        for (size_t i = 0; i < size; ++i) {
            if (i > 0) {
                oss << ", ";
            }
            oss << formatElement(buf[i]);
        }
        oss << ']';
        return oss.str();
    }

    template <typename T>
    inline std::string arrayStringify(const std::vector<T> &arr) {
        return arrayStringify(arr.data(), arr.size());
    }

    template <>
    inline std::string arrayStringify(const std::vector<bool> &arr) {
        std::ostringstream oss;
        oss << '[';
        bool isFirstFlag = true;
        for (const auto &item : arr) {
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

    template <typename T>
    inline const char *typeToString() {
        static_assert(std::is_same_v<T, float> ||
                      std::is_same_v<T, int64_t> ||
                      std::is_same_v<T, bool>,
                      "T must be one of the following types: float, int64_t, bool.");
        const char *elemType;
        if constexpr (std::is_same_v<T, float>) {
            elemType = "float";
        } else if constexpr (std::is_same_v<T, int64_t>) {
            elemType = "int64";
        } else if constexpr (std::is_same_v<T, bool>) {
            elemType = "bool";
        } else {
            elemType = "";
        }
        return elemType;
    }

    template <typename T>
    inline dsinfer::JsonValue toInputDataArray(const char *name, const T *buffer, size_t size) {
        static_assert(std::is_same_v<T, float> ||
                      std::is_same_v<T, int64_t> ||
                      std::is_same_v<T, bool>,
                      "T must be one of the following types: float, int64_t, bool.");
        auto elemType = typeToString<T>();

        dsinfer::JsonArray value;
        value.reserve(size);
        for (size_t i = 0; i < size; ++i) {
            value.emplace_back(buffer[i]);
        }
        return dsinfer::JsonObject {
            {"name", name},
            {"format", "array"},
            {"data", dsinfer::JsonObject {
                {"type", elemType},
                {"shape", dsinfer::JsonArray {int64_t(1), int64_t(size)}},
                {"value", value}
            }}
        };
    }

    template <typename T>
    inline dsinfer::JsonValue toInputDataBytes(const char *name, const T *buffer, size_t size) {
        static_assert(std::is_same_v<T, float> ||
                          std::is_same_v<T, int64_t> ||
                          std::is_same_v<T, bool>,
                      "T must be one of the following types: float, int64_t, bool.");
        auto elemType = typeToString<T>();

        const uint8_t *bytesBuffer = reinterpret_cast<const uint8_t *>(buffer);
        size_t bytesCount = size * sizeof(T);

        return dsinfer::JsonObject {
            {"name", name},
            {"format", "bytes"},
            {"data", dsinfer::JsonObject {
                 {"type", elemType},
                 {"shape", dsinfer::JsonArray {int64_t(1), int64_t(size)}},
                 {"value", dsinfer::JsonValue(std::vector<uint8_t>(
                               bytesBuffer, bytesBuffer + bytesCount))}
            }}
        };
    }

    template <typename T>
    inline dsinfer::JsonValue toContextObj(const T *data, size_t size) {
        static_assert(std::is_same_v<T, float> ||
                      std::is_same_v<T, int64_t> ||
                      std::is_same_v<T, bool>,
                      "T must be one of the following types: float, int64_t, bool.");
        auto buffer = reinterpret_cast<const uint8_t *>(data);
        if (size > std::numeric_limits<int64_t>::max()) {
            size = std::numeric_limits<int64_t>::max();
        }
        auto bufferSize = size * sizeof(T);
        return dsinfer::JsonObject {
            {"type",    "object"},
            {"content", dsinfer::JsonObject{
                 {"class", "Ort::Value"},
                 {"format", "bytes"},
                 {"data", dsinfer::JsonObject{{"type", typeToString<T>()},
                     {"shape", dsinfer::JsonArray{int64_t(1), int64_t(size)}},
                     {"value", std::vector<uint8_t>(buffer, buffer + bufferSize)}}}}}
        };
    }

    template <typename T>
    inline dsinfer::JsonValue toContextObj(const std::vector<T> &inputData) {
        return toContextObj(inputData.data(), inputData.size());
    }

    template <>
    inline dsinfer::JsonValue toContextObj(const std::vector<bool> &inputData) {
        std::vector<uint8_t> temp(inputData.begin(), inputData.end());
        return toContextObj<bool>(reinterpret_cast<bool *>(temp.data()), temp.size());
    }
}

#endif // TST_ONNXDRIVER_VALUEUTILS_H