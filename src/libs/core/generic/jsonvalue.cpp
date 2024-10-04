#include "jsonvalue.h"

#include <nlohmann/json.hpp>

namespace dsinfer {

    class JsonValueContainter {
    public:
        nlohmann::json json;
    };

    JsonValue::JsonValue(Type type) : _data(new JsonValueContainter()) {
        auto &json = _data->json;
        switch (type) {
            case Null: {
                json = nullptr;
                break;
            }
            case Bool: {
                json = false;
                break;
            }
            case Double: {
                json = 0.0;
                break;
            }
            case String: {
                json = std::string();
                break;
            }
            case Binary:
                json = std::vector<uint8_t>();
                break;
            case Array: {
                json = nlohmann::json::array();
                break;
            }
            case Object: {
                json = nlohmann::json::object();
                break;
            }
            case Undefined: {
                json = nullptr;
                break;
            }
        }
    }

    JsonValue::JsonValue(bool b) : _data(new JsonValueContainter()) {
        auto &json = _data->json;
        json = b;
    }

    JsonValue::JsonValue(double n) : _data(new JsonValueContainter()) {
        auto &json = _data->json;
        json = n;
    }

    JsonValue::JsonValue(int n) : _data(new JsonValueContainter()) {
        auto &json = _data->json;
        json = n;
    }

    JsonValue::JsonValue(const std::string &s) : _data(new JsonValueContainter()) {
        auto &json = _data->json;
        json = s;
    }

    JsonValue::JsonValue(const char *s) : _data(new JsonValueContainter()) {
        auto &json = _data->json;
        json = s;
    }

    JsonValue::JsonValue(const std::vector<uint8_t> &bytes) : _data(new JsonValueContainter()) {
        auto &json = _data->json;
        json = bytes;
    }

    JsonValue::JsonValue(const uint8_t *data, int size) : _data(new JsonValueContainter()) {
        auto &json = _data->json;
        json = std::vector<uint8_t>(data, data + size);
    }

    JsonValue::JsonValue(const JsonValue::_Array &a) : _data(new JsonValueContainter()) {
        auto &json = _data->json;
        for (const auto &item : a) {
            json.push_back(item._data->json);
        }
    }

    JsonValue::JsonValue(const JsonValue::_Object &o) : _data(new JsonValueContainter()) {
        auto &json = _data->json;
        for (const auto &it : o) {
            json[it.first] = it.second._data->json;
        }
    }

    JsonValue::~JsonValue() = default;

    JsonValue::JsonValue(const JsonValue &other) = default;

    JsonValue &JsonValue::operator=(const JsonValue &other) = default;

    JsonValue::JsonValue(JsonValue &&other) noexcept = default;

    JsonValue::Type JsonValue::type() const {
        JsonValue::Type type = Undefined;
        switch (_data->json.type()) {
            case nlohmann::json_abi_v3_11_3::detail::value_t::null:
                type = Null;
                break;
            case nlohmann::json_abi_v3_11_3::detail::value_t::object:
                type = Object;
                break;
            case nlohmann::json_abi_v3_11_3::detail::value_t::array:
                type = Array;
                break;
            case nlohmann::json_abi_v3_11_3::detail::value_t::string:
                type = String;
                break;
            case nlohmann::json_abi_v3_11_3::detail::value_t::boolean:
                type = Bool;
                break;
            case nlohmann::json_abi_v3_11_3::detail::value_t::number_integer:
            case nlohmann::json_abi_v3_11_3::detail::value_t::number_unsigned:
            case nlohmann::json_abi_v3_11_3::detail::value_t::number_float:
                type = Double;
                break;
            case nlohmann::detail::value_t::binary:
                type = Binary;
                break;
            default:
                break;
        }
        return type;
    }
    bool JsonValue::toBool(bool defaultValue) const {
        auto &json = _data->json;
        if (json.is_boolean()) {
            return json.get<bool>();
        }
        return false;
    }
    int JsonValue::toInt(int defaultValue) const {
        auto &json = _data->json;
        switch (json.type()) {
            case nlohmann::json_abi_v3_11_3::detail::value_t::number_integer:
                return json.get<int>();
            case nlohmann::json_abi_v3_11_3::detail::value_t::number_unsigned:
                return int(json.get<unsigned int>());
            case nlohmann::json_abi_v3_11_3::detail::value_t::number_float:
                return int(json.get<double>());
            default:
                break;
        }
        return 0;
    }
    double JsonValue::toDouble(double defaultValue) const {
        auto &json = _data->json;
        switch (json.type()) {
            case nlohmann::json_abi_v3_11_3::detail::value_t::number_integer:
                return json.get<int>();
            case nlohmann::json_abi_v3_11_3::detail::value_t::number_unsigned:
                return json.get<unsigned int>();
            case nlohmann::json_abi_v3_11_3::detail::value_t::number_float:
                return json.get<double>();
            default:
                break;
        }
        return 0;
    }
    std::string JsonValue::toString(const std::string &defaultValue) const {
        auto &json = _data->json;
        if (json.is_string()) {
            return json.get<std::string>();
        }
        return {};
    }
    std::vector<uint8_t> JsonValue::toBinary(const std::vector<uint8_t> &defaultValue) const {
        auto &json = _data->json;
        if (json.is_binary()) {
            return json.get<std::vector<uint8_t>>();
        }
        return {};
    }
    JsonValue::_Array JsonValue::toArray(const JsonValue::_Array &defaultValue) const {
        auto &json = _data->json;
        if (json.is_array()) {
            _Array a;
            a.reserve(json.size());
            for (const auto &item : json) {
                JsonValue val;
                val._data->json = item;
                a.push_back(val);
            }
            return a;
        }
        return {};
    }
    JsonValue::_Object JsonValue::toObject(const JsonValue::_Object &defaultValue) const {
        auto &json = _data->json;
        if (json.is_object()) {
            _Object o;
            for (auto it = json.begin(); it != json.end(); ++it) {
                JsonValue val;
                val._data->json = it.value();
                o[it.key()] = val;
            }
            return o;
        }
        return {};
    }
    JsonValue JsonValue::operator[](const char *key) const {
        auto &json = _data->json;
        if (json.is_object()) {
            auto it = json.find(key);
            if (it == json.end()) {
                return {};
            }
            JsonValue val;
            val._data->json = it.value();
            return val;
        }
        return {};
    }
    JsonValue JsonValue::operator[](int i) const {
        auto &json = _data->json;
        if (json.is_array()) {
            JsonValue val;
            val._data->json = json[i];
            return val;
        }
        return {};
    }
    bool JsonValue::operator==(const JsonValue &other) const {
        return _data->json == other._data->json;
    }
    std::string JsonValue::toJson(int indent) const {
        return _data->json.dump(indent);
    }
    JsonValue JsonValue::fromJson(const std::string &json, bool ignore_comments,
                                  std::string *error) {
        JsonValue val;
        try {
            auto ex = nlohmann::json::parse(json, nullptr, true, ignore_comments);
            val._data->json = ex;
        } catch (const std::exception &e) {
            if (error)
                *error = e.what();
        }
        return val;
    }

}