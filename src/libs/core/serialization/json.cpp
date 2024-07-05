#include  "json_cxxapi.h"

#include <cstring>

static dsinfer_json_bridge g_bridge;

void dsinfer_json_string_create(dsinfer_json_string *s, const char *data, int managed) {
    int size = strlen(data);
    s->data = new char[size + 1];
    for (int i = 0; i < size; ++i) {
        s->data[i] = data[i];
    }
    s->size = size;
    s->managed = managed;
}

void dsinfer_json_string_destroy(dsinfer_json_string *s) {
    if (s->data && !s->managed) {
        delete[] s->data;
    }
}

void dsinfer_json_bridge_global_set(const dsinfer_json_bridge *b) {
    g_bridge = *b;
}

void dsinfer_json_bridge_global_get(dsinfer_json_bridge *b) {
    *b = g_bridge;
}

// Copy `dsinfer_json_string` to `std::string` and free the buffer.
static std::string dsinfer_json_string_to_std_string(dsinfer_json_string *s) {
    if (s->data == nullptr) {
        return {};
    }
    std::string res(s->data, s->size);
    dsinfer_json_string_destroy(s);
    return res;
}

namespace dsinfer {

    // Value
    JsonValue::JsonValue(JsonValue::Type type) {
        g_bridge.value_create(&_data, type, nullptr);
    }
    JsonValue::JsonValue(bool b) {
        g_bridge.value_create(&_data, Bool, &b);
    }
    JsonValue::JsonValue(double n) {
        g_bridge.value_create(&_data, Double, &n);
    }
    JsonValue::JsonValue(int n) {
        double d = n;
        g_bridge.value_create(&_data, Double, &d);
    }
    JsonValue::JsonValue(const std::string &s) {
        g_bridge.value_create(&_data, String, s.data());
    }
    JsonValue::JsonValue(const char *s) {
        g_bridge.value_create(&_data, String, s);
    }
    JsonValue::JsonValue(const JsonArray &a) {
        g_bridge.value_create(&_data, Array, &a._data);
    }
    JsonValue::JsonValue(const JsonObject &o) {
        g_bridge.value_create(&_data, Object, &o._data);
    }
    JsonValue::~JsonValue() {
        if (_data.data != nullptr) {
            g_bridge.value_destroy(&_data);
        }
    }
    JsonValue::JsonValue(const JsonValue &other) {
        g_bridge.value_copy(&other._data, &_data);
    }
    JsonValue &JsonValue::operator=(const JsonValue &other) {
        if (&other == this) {
            return *this;
        }
        g_bridge.value_destroy(&_data);
        g_bridge.value_copy(&other._data, &_data);
        return *this;
    }
    JsonValue::JsonValue(JsonValue &&other) noexcept : _data(other._data) {
        other._data.data = nullptr;
        other._data.type = Undefined;
    }
    bool JsonValue::toBool(bool defaultValue) const {
        if (_data.type != Bool) {
            return defaultValue;
        }
        bool res;
        g_bridge.value_get(&_data, &res);
        return res;
    }
    int JsonValue::toInt(int defaultValue) const {
        return int(toDouble(defaultValue));
    }
    double JsonValue::toDouble(double defaultValue) const {
        if (_data.type != Double) {
            return defaultValue;
        }
        double res;
        g_bridge.value_get(&_data, &res);
        return res;
    }
    std::string JsonValue::toString(const std::string &defaultValue) const {
        if (_data.type != Double) {
            return defaultValue;
        }
        dsinfer_json_string res;
        g_bridge.value_get(&_data, &res);
        return dsinfer_json_string_to_std_string(&res);
    }
    JsonArray JsonValue::toArray() const {
        return toArray({});
    }
    JsonArray JsonValue::toArray(const JsonArray &defaultValue) const {
        if (_data.type != Array) {
            return defaultValue;
        }
        dsinfer_json_array res;
        g_bridge.value_get(&_data, &res);
        return res;
    }
    JsonObject JsonValue::toObject() const {
        return toObject({});
    }
    JsonObject JsonValue::toObject(const JsonObject &defaultValue) const {
        if (_data.type != Object) {
            return defaultValue;
        }
        dsinfer_json_object res;
        g_bridge.value_get(&_data, &res);
        return res;
    }
    JsonValue JsonValue::operator[](const char *key) const {
        if (_data.type != Object) {
            return {};
        }
        return toObject().value(key);
    }
    JsonValue JsonValue::operator[](int i) const {
        if (_data.type != Array) {
            return {};
        }
        return toArray().at(i);
    }
    bool JsonValue::operator==(const JsonValue &other) const {
        return g_bridge.value_equal(&_data, &other._data);
    }
    JsonValue::JsonValue(dsinfer_json_value &&data) : _data(data) {
    }
    JsonValueConstRef::JsonValueConstRef(const JsonValueConstRef &other) {
        if (other.is_object) {
            g_bridge.object_iterator_copy(&other.it, &it);
        } else {
            arr = other.arr;
            idx = other.idx;
        }
    }
    JsonValueConstRef::~JsonValueConstRef() {
        if (is_object) {
            g_bridge.object_iterator_destroy(&it);
        }
    }
    JsonArray JsonValueConstRef::toArray() const {
        return toArray({});
    }
    JsonArray JsonValueConstRef::toArray(const JsonArray &defaultValue) const {
        return toValue().toArray(defaultValue);
    }
    JsonObject JsonValueConstRef::toObject() const {
        return toObject({});
    }
    JsonObject JsonValueConstRef::toObject(const JsonObject &defaultValue) const {
        return toValue().toObject(defaultValue);
    }
    JsonValue JsonValueConstRef::toValue() const {
        if (is_object) {
            dsinfer_json_value res;
            g_bridge.object_iterator_get(&it, &res);
            return res;
        }
        dsinfer_json_value res;
        g_bridge.array_get(arr, idx, &res);
        return res;
    }
    void JsonValueConstRef::rebind(const JsonValueConstRef &other) {
        if (is_object) {
            g_bridge.object_iterator_destroy(&it);
        }

        if (other.is_object) {
            is_object = true;
            g_bridge.object_iterator_copy(&other.it, &it);
        } else {
            is_object = false;
            arr = other.arr;
            idx = other.idx;
        }
    }
    bool JsonValueConstRef::equal(const JsonValueConstRef &other) const {
        if (other.is_object != is_object) {
            return false;
        }
        if (is_object) {
            return g_bridge.object_iterator_equal(&it, &other.it);
        }
        return arr == other.arr && idx == other.idx;
    }
    std::string JsonValueConstRef::objectKey() const {
        dsinfer_json_string res;
        g_bridge.object_iterator_key(&it, &res);
        return dsinfer_json_string_to_std_string(&res);
    }
    void JsonValueConstRef::objectIteratorNext() {
        g_bridge.object_iterator_next(&it);
    }
    void JsonValueConstRef::objectIteratorPrev() {
        g_bridge.object_iterator_prev(&it);
    }
    JsonValueConstRef::JsonValueConstRef(dsinfer_json_array *array, int idx) : is_object(false) {
        this->arr = array;
        this->idx = idx;
    }
    JsonValueConstRef::JsonValueConstRef(dsinfer_json_object_iterator &&it) : is_object(true) {
        this->it = it;
    }
    JsonValueRef::JsonValueRef(const JsonValueRef &other) = default;
    JsonValueRef &JsonValueRef::operator=(const JsonValue &val) {
        if (is_object) {
            g_bridge.object_iterator_set(&it, &val._data);
        } else {
            g_bridge.array_replace(arr, idx, &val._data);
        }
        return *this;
    }
    JsonValueRef &JsonValueRef::operator=(const JsonValueRef &val) {
        std::ignore = operator=(val.toValue());
        return *this;
    }





    // Array
    JsonArray::JsonArray() {
        g_bridge.array_create(&_data);
    }
    JsonArray::JsonArray(std::initializer_list<JsonValue> args) {
        g_bridge.array_create(&_data);
        for (const auto &arg : args)
            append(arg);
    }
    JsonArray::~JsonArray() {
        if (_data.data) {
            g_bridge.array_destroy(&_data);
        }
    }
    JsonArray::JsonArray(const JsonArray &other) {
        g_bridge.array_copy(&other._data, &_data);
    }
    JsonArray &JsonArray::operator=(const JsonArray &other) {
        if (&other == this) {
            return *this;
        }
        g_bridge.array_destroy(&_data);
        g_bridge.array_copy(&other._data, &_data);
        return *this;
    }
    JsonArray::JsonArray(JsonArray &&other) noexcept : _data(other._data) {
        other._data.size = 0;
        other._data.data = nullptr;
    }
    JsonValue JsonArray::at(int i) const {
        dsinfer_json_value res;
        g_bridge.array_get(&_data, i, &res);
        return res;
    }
    void JsonArray::removeAt(int i) {
        g_bridge.array_remove(&_data, i);
    }
    JsonValue JsonArray::takeAt(int i) {
        auto res = at(i);
        removeAt(i);
        return res;
    }
    void JsonArray::insert(int i, const JsonValue &value) {
        g_bridge.array_insert(&_data, i, &value._data);
    }
    void JsonArray::replace(int i, const JsonValue &value) {
        g_bridge.array_replace(&_data, i, &value._data);
    }
    bool JsonArray::contains(const JsonValue &element) const {
        for (int i = 0; i < size(); i++) {
            if (at(i) == element) {
                return true;
            }
        }
        return false;
    }
    JsonValueRef JsonArray::operator[](int i) {
        return JsonValueRef(&_data, i);
    }
    JsonValue JsonArray::operator[](int i) const {
        return at(i);
    }
    bool JsonArray::operator==(const JsonArray &other) const {
        return g_bridge.array_equal(&_data, &other._data);
    }
    JsonArray::JsonArray(dsinfer_json_array &&data) : _data(data) {
    }





    // Object
    JsonObject::JsonObject() {
        g_bridge.object_create(&_data);
    }
    JsonObject::JsonObject(std::initializer_list<std::pair<std::string, JsonValue>> args) {
        g_bridge.object_create(&_data);
        for (const auto &arg : args)
            insert(arg.first, arg.second);
    }
    JsonObject::~JsonObject() {
        if (_data.data) {
            g_bridge.object_destroy(&_data);
        }
    }
    JsonObject::JsonObject(const JsonObject &other) noexcept {
        g_bridge.object_copy(&other._data, &_data);
    }
    JsonObject &JsonObject::operator=(const JsonObject &other) noexcept {
        if (&other == this) {
            return *this;
        }
        g_bridge.object_destroy(&_data);
        g_bridge.object_copy(&other._data, &_data);
        return *this;
    }
    JsonObject::JsonObject(JsonObject &&other) noexcept : _data(other._data) {
        other._data.data = nullptr;
        other._data.size = 0;
    }
    std::vector<std::string> JsonObject::keys() const {
        std::vector<std::string> res;
        res.reserve(size());
        for (auto it = begin(); it != end(); ++it) {
            res.push_back(it.key());
        }
        return res;
    }
    JsonValue JsonObject::value(const char *key) const {
        dsinfer_json_object_iterator it;
        g_bridge.object_find(&_data, key, &it);
        dsinfer_json_value res;
        g_bridge.object_iterator_get(&it, &res);
        g_bridge.object_iterator_destroy(&it);
        return res;
    }
    JsonValueRef JsonObject::operator[](const char *key) {
        dsinfer_json_object_iterator it;
        g_bridge.object_find(&_data, key, &it);
        return it;
    }
    void JsonObject::remove(const char *key) {
        auto it = find(key);
        if (it == end()) {
            return;
        }
        erase(it);
    }
    JsonValue JsonObject::take(const char *key) {
        auto it = find(key);
        if (it == end()) {
            return {};
        }
        auto res = it.value();
        erase(it);
        return res;
    }
    bool JsonObject::operator==(const JsonObject &other) const {
        return g_bridge.object_equal(&_data, &other._data);
    }

    JsonObject::iterator JsonObject::begin() {
        dsinfer_json_object_iterator it;
        g_bridge.object_begin(&_data, &it);
        return it;
    }
    JsonObject::const_iterator JsonObject::constBegin() const {
        return begin();
    }
    JsonObject::iterator JsonObject::end() {
        dsinfer_json_object_iterator it;
        g_bridge.object_end(&_data, &it);
        return it;
    }
    JsonObject::const_iterator JsonObject::constEnd() const {
        return end();
    }
    JsonObject::iterator JsonObject::erase(iterator it) {
        g_bridge.object_erase(&_data, &it.item.it);
        return it;
    }
    JsonObject::iterator JsonObject::find(const char *key) {
        dsinfer_json_object_iterator it;
        g_bridge.object_find(&_data, key, &it);
        return it;
    }
    JsonObject::const_iterator JsonObject::constFind(const char *key) const {
        return find(key);
    }
    JsonObject::iterator JsonObject::insert(const char *key, const JsonValue &value) {
        dsinfer_json_object_iterator it;
        g_bridge.object_insert(&_data, key, &value._data, &it);
        return it;
    }

    JsonObject::JsonObject(dsinfer_json_object &&data) : _data(data) {
    }





    // Document
    JsonDocument::JsonDocument() {
        g_bridge.value_create(&_data, JsonValue::Null, nullptr);
    }
    JsonDocument::JsonDocument(const JsonObject &object) {
        g_bridge.value_create(&_data, JsonValue::Object, &object._data);
    }
    JsonDocument::JsonDocument(const JsonArray &array) {
        g_bridge.value_create(&_data, JsonValue::Array, &array._data);
    }
    JsonDocument::~JsonDocument() {
        if (_data.data) {
            g_bridge.value_destroy(&_data);
        }
    }
    JsonDocument::JsonDocument(const JsonDocument &other) {
        g_bridge.value_copy(&other._data, &_data);
    }
    JsonDocument &JsonDocument::operator=(const JsonDocument &other) {
        if (&other == this) {
            return *this;
        }
        g_bridge.value_destroy(&_data);
        g_bridge.value_copy(&other._data, &_data);
        return *this;
    }
    JsonDocument::JsonDocument(JsonDocument &&other) noexcept : _data(other._data) {
        other._data.data = nullptr;
        other._data.type = JsonValue::Undefined;
    }
    JsonDocument JsonDocument::fromJson(const char *json, std::string *error) {
        dsinfer_json_value doc;
        dsinfer_json_string err;
        g_bridge.json_deserialize(&doc, &err);
        if (error) {
            *error = dsinfer_json_string_to_std_string(&err);
        } else {
            dsinfer_json_string_destroy(&err);
        }
        return doc;
    }
    std::string JsonDocument::toJson(int options) const {
        switch (_data.type) {
            case JsonValue::Array:
            case JsonValue::Object: {
                dsinfer_json_string res;
                g_bridge.json_serialize(&_data, &res, options);
                return dsinfer_json_string_to_std_string(&res);
            }
            default:
                break;
        }
        return {};
    }
    JsonDocument JsonDocument::fromCbor(const char *cbor, std::string *error) {
        dsinfer_json_value doc;
        dsinfer_json_string err;
        g_bridge.cbor_deserialize(&doc, &err);
        if (error) {
            *error = dsinfer_json_string_to_std_string(&err);
        } else {
            dsinfer_json_string_destroy(&err);
        }
        return doc;
    }
    std::string JsonDocument::toCbor(int options) const {
        switch (_data.type) {
            case JsonValue::Array:
            case JsonValue::Object: {
                dsinfer_json_string res;
                g_bridge.cbor_serialize(&_data, &res, options);
                return dsinfer_json_string_to_std_string(&res);
            }
            default:
                break;
        }
        return {};
    }
    bool JsonDocument::isEmpty() const {
        switch (_data.type) {
            case JsonValue::Array: {
                return array().isEmpty();
            }
            case JsonValue::Object: {
                return object().isEmpty();
            }
            default:
                break;
        }
        return true;
    }
    bool JsonDocument::isArray() const {
        return _data.type == JsonValue::Array;
    }
    bool JsonDocument::isObject() const {
        return _data.type == JsonValue::Object;
    }
    bool JsonDocument::isNull() const {
        return _data.type == JsonValue::Null || _data.type == JsonValue::Undefined;
    }
    JsonObject JsonDocument::object() const {
        dsinfer_json_object o;
        g_bridge.value_get(&_data, &o);
        return o;
    }
    JsonArray JsonDocument::array() const {
        dsinfer_json_array a;
        g_bridge.value_get(&_data, &a);
        return a;
    }
    void JsonDocument::setObject(const JsonObject &object) {
        g_bridge.value_destroy(&_data);
        g_bridge.value_create(&_data, JsonValue::Object, &object._data);
    }
    void JsonDocument::setArray(const JsonArray &array) {
        g_bridge.value_destroy(&_data);
        g_bridge.value_create(&_data, JsonValue::Array, &array._data);
    }
    JsonDocument::JsonDocument(dsinfer_json_value &&data) : _data(data) {
    }

}