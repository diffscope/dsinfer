#ifndef JSONVALUE_H
#define JSONVALUE_H

#include <map>
#include <string>
#include <vector>

#include <dsinfer/dsinferglobal.h>

namespace dsinfer {

    class JsonValueContainer;

    class DSINFER_EXPORT JsonValue {
    public:
        enum Type {
            Null = 0x0,
            Bool = 0x1,
            Integer = 0x2,
            Double = 0x3,
            String = 0x4,
            Binary = 0x5,
            Array = 0x11,
            Object = 0x12,
            Undefined = 0x80
        };

        using _Object = std::map<std::string, JsonValue>;
        using _Array = std::vector<JsonValue>;

        JsonValue(Type type = Null);
        JsonValue(bool b);
        JsonValue(double n);
        JsonValue(int n);
        JsonValue(int64_t n);
        JsonValue(const std::string &s);
        JsonValue(const char *s);
        JsonValue(const std::vector<uint8_t> &bytes);
        JsonValue(const uint8_t *data, int size);
        JsonValue(const _Array &a);
        JsonValue(const _Object &o);
        ~JsonValue();

        JsonValue(const JsonValue &other);
        JsonValue &operator=(const JsonValue &other);

        JsonValue(JsonValue &&other) noexcept;
        JsonValue &operator=(JsonValue &&other) noexcept {
            swap(other);
            return *this;
        }
        void swap(JsonValue &other) noexcept {
            std::swap(_data, other._data);
        }

    public:
        Type type() const;
        inline bool isNull() const {
            return type() == Null;
        }
        inline bool isBool() const {
            return type() == Bool;
        }
        inline bool isInt() const {
            return type() == Integer;
        }
        inline bool isDouble() const {
            return type() == Double;
        }
        inline bool isString() const {
            return type() == String;
        }
        inline bool isBinary() const {
            return type() == Binary;
        }
        inline bool isArray() const {
            return type() == Array;
        }
        inline bool isObject() const {
            return type() == Object;
        }
        inline bool isUndefined() const {
            return type() == Undefined;
        }

        bool toBool(bool defaultValue = false) const;
        int toInt(int defaultValue = 0) const;
        int64_t toInt64(int64_t defaultValue = 0) const;
        double toDouble(double defaultValue = 0) const;
        std::string toString(const std::string &defaultValue = {}) const;
        std::vector<uint8_t> toBinary(const std::vector<uint8_t> &defaultValue = {}) const;
        _Array toArray(const _Array &defaultValue = {}) const;
        _Object toObject(const _Object &defaultValue = {}) const;

        inline JsonValue operator[](const std::string &key) const {
            return operator[](key.c_str());
        }
        inline JsonValue operator[](std::string_view key) const {
            return operator[](key.data());
        }
        JsonValue operator[](const char *key) const;
        JsonValue operator[](int i) const;

        bool operator==(const JsonValue &other) const;
        inline bool operator!=(const JsonValue &other) const {
            return !(*this == other);
        }

    public:
        std::string toJson(int indent = -1) const;
        static JsonValue fromJson(const std::string &json, bool ignore_comments,
                                  std::string *error = nullptr);

        std::vector<uint8_t> toCbor() const;
        static JsonValue fromCbor(const std::vector<uint8_t> &cbor, std::string *error = nullptr);

    protected:
        std::shared_ptr<JsonValueContainer> _data;
    };

    using JsonObject = JsonValue::_Object;

    using JsonArray = JsonValue::_Array;

}

#endif // JSONVALUE_H
