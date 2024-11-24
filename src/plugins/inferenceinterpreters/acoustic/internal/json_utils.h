#ifndef JSON_VALIDATOR_H
#define JSON_VALIDATOR_H

#include <string>
#include <type_traits>
#include <dsinfer/error.h>
#include <dsinfer/jsonvalue.h>

namespace dsinfer {
    template <typename, typename = void>
    struct is_object_like : std::false_type {};

    // Specialization for types that have key-value semantics
    template <typename T>
    struct is_object_like<T, std::void_t<
        typename T::key_type,                // Type has a key_type
        typename T::mapped_type,             // Type has a mapped_type
        decltype(std::declval<T>()["key"]),  // Supports operator[]
        decltype(std::begin(std::declval<T>())), // Iterable
        decltype(std::end(std::declval<T>()))    // Iterable
    >> : std::true_type {};

    // Helper variable template
    template <typename T>
    inline constexpr bool is_object_like_v = is_object_like<T>::value;

    template <typename T>
    inline const char *type_to_string() {
        if constexpr (std::is_same_v<T, std::string>) {
            return "string";
        } else if constexpr (std::is_same_v<T, bool>) {
            return "boolean";
        } else if constexpr (std::is_integral_v<T>) {
            return "integer";
        } else if constexpr (std::is_floating_point_v<T>) {
            return "double";
        } else if constexpr (std::is_array_v<T>) {
            return "array";
        } else if constexpr (is_object_like_v<T>) {
            return "object";
        } else {
            return "";
        }
    }

    template <typename T>
    inline bool is_json_type(const JsonValue &value) {
        if constexpr (std::is_same_v<T, std::string>) {
            return value.isString();
        }  else if constexpr (std::is_same_v<T, bool>) {
            return value.isBool();
        } else if constexpr (std::is_integral_v<T>) {
            return value.isInt();
        } else if constexpr (std::is_floating_point_v<T>) {
            return value.isDouble() || value.isInt();
        } else if constexpr (std::is_array_v<T>) {
            return value.isArray();
        } else if constexpr (is_object_like_v<T>) {
            return value.isObject();
        } else {
            return false;
        }
    }

    template <typename T>
    inline T to_json_type(const JsonValue &value) {
        if constexpr (std::is_same_v<T, std::string>) {
            return value.toString();
        } else if constexpr (std::is_same_v<T, bool>) {
            return value.toBool();
        } else if constexpr (std::is_integral_v<T>) {
            if constexpr (std::is_same_v<T, int>) {
                return value.toInt();
            }
            return value.toInt64();
        } else if constexpr (std::is_floating_point_v<T>) {
            return value.toDouble();
        } else {
            return {};
        }
    }

    template<typename T>
    inline bool get_input(const std::string &className, const JsonValue &json, const std::string &key, Error *error,
                         T &outValue) {
        const auto val = json[key];
        if (val.isUndefined()) {
            if (error) {
                *error = Error(Error::InvalidFormat,
                "Invalid format of " + className + ": missing key \"" + key + "\" of " + type_to_string<T>() +
                "value");
            }
            return false;
        }
        if (!is_json_type<T>(val)) {
            if (error) {
                *error = Error(Error::InvalidFormat,
                               "Invalid format of " + className + ": key \"" + key +
                               "\" is not " + type_to_string<T>());
            }
            return false;
        }
        outValue = to_json_type<T>(val);
        return true;
    }

    template<typename T>
    inline JsonArray to_json_array(const std::vector<T> &container) {
        JsonArray array;
        array.reserve(container.size());
        for (const auto &item: container) {
            if constexpr (std::is_constructible_v<JsonValue, T>) {
                array.emplace_back(item);
            } else {
                array.emplace_back(to_json(item));
            }
        }
        return array;
    }
}

#endif // JSON_VALIDATOR_H