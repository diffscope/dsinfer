#ifndef JSONAPI_H
#define JSONAPI_H

#include <string>
#include <memory>
#include <vector>
#include <string_view>

#include <dsinferCore/jsonapi_capi.h>

namespace JsonApi {

    class JsonValueConstRef;

    class JsonValueRef;

    class JsonArray;

    class JsonObject;

    class JsonDocument;

    class DSINFER_CORE_EXPORT JsonValue {
    public:
        enum Type {
            Null = 0x0,
            Bool = 0x1,
            Double = 0x2,
            String = 0x3,
            Array = 0x4,
            Object = 0x5,
            Undefined = 0x80
        };

        JsonValue(Type type = Null);
        JsonValue(bool b);
        JsonValue(double n);
        JsonValue(int n);
        JsonValue(const std::string &s);
        JsonValue(const char *s);
        JsonValue(const JsonArray &a);
        JsonValue(const JsonObject &o);
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
        inline Type type() const {
            return static_cast<Type>(_data.type);
        }
        inline bool isNull() const {
            return type() == Null;
        }
        inline bool isBool() const {
            return type() == Bool;
        }
        inline bool isDouble() const {
            return type() == Double;
        }
        inline bool isString() const {
            return type() == String;
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
        double toDouble(double defaultValue = 0) const;
        std::string toString(const std::string &defaultValue = {}) const;
        JsonArray toArray() const;
        JsonArray toArray(const JsonArray &defaultValue) const;
        JsonObject toObject() const;
        JsonObject toObject(const JsonObject &defaultValue) const;

        inline JsonValue operator[](const std::string &key) const {
            return operator[](key.data());
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

    protected:
        JsonValue(jsonapi_value &&data);
        jsonapi_value _data;

        friend class JsonValueConstRef;
        friend class JsonValueRef;
        friend class JsonArray;
        friend class JsonObject;
        friend class JsonDocument;
    };

    class DSINFER_CORE_EXPORT JsonValueConstRef {
    public:
        JsonValueConstRef(const JsonValueConstRef &other);
        JsonValueConstRef &operator=(const JsonValueConstRef &) = delete;
        ~JsonValueConstRef();

    public:
        inline operator JsonValue() const {
            return toValue();
        }
        JsonValue::Type type() const {
            return toValue().type();
        }
        bool isNull() const {
            return type() == JsonValue::Null;
        }
        bool isBool() const {
            return type() == JsonValue::Bool;
        }
        bool isDouble() const {
            return type() == JsonValue::Double;
        }
        bool isString() const {
            return type() == JsonValue::String;
        }
        bool isArray() const {
            return type() == JsonValue::Array;
        }
        bool isObject() const {
            return type() == JsonValue::Object;
        }
        bool isUndefined() const {
            return type() == JsonValue::Undefined;
        }
        bool toBool(bool defaultValue = false) const {
            return toValue().toBool(defaultValue);
        }
        int toInt(int defaultValue = 0) const {
            return toValue().toInt(defaultValue);
        }
        double toDouble(double defaultValue = 0) const {
            return toValue().toDouble(defaultValue);
        }
        std::string toString(const std::string &defaultValue = {}) const {
            return toValue().toString(defaultValue);
        }
        JsonArray toArray() const;
        JsonArray toArray(const JsonArray &defaultValue) const;
        JsonObject toObject() const;
        JsonObject toObject(const JsonObject &defaultValue) const;

        JsonValue operator[](std::string_view key) const {
            return toValue()[key];
        }
        JsonValue operator[](const char *key) const {
            return toValue()[key];
        }
        JsonValue operator[](int i) const {
            return toValue()[i];
        }
        inline bool operator==(const JsonValue &other) const {
            return toValue() == other;
        }
        inline bool operator!=(const JsonValue &other) const {
            return toValue() != other;
        }

    protected:
        JsonValue toValue() const;

        void rebind(const JsonValueConstRef &other);
        bool equal(const JsonValueConstRef &other) const;

        std::string objectKey() const;
        void objectIteratorNext();
        void objectIteratorPrev();

        JsonValueConstRef(jsonapi_array *array, int idx);
        JsonValueConstRef(jsonapi_object_iterator &&it);

        union {
            struct {
                jsonapi_array *arr;
                int idx;
            };
            jsonapi_object_iterator it;
        };
        bool is_object;

        friend class JsonValue;
        friend class JsonArray;
        friend class JsonObject;
        friend class JsonDocument;
    };

    class DSINFER_CORE_EXPORT JsonValueRef : public JsonValueConstRef {
    public:
        JsonValueRef(const JsonValueRef &other);
        JsonValueRef &operator=(const JsonValue &val);
        JsonValueRef &operator=(const JsonValueRef &val);

    protected:
        JsonValueRef(jsonapi_array *array, int idx) : JsonValueConstRef(array, idx) {
        }
        JsonValueRef(jsonapi_object_iterator &&it)
            : JsonValueConstRef(std::forward<jsonapi_object_iterator>(it)) {
        }

        friend class JsonValue;
        friend class JsonArray;
        friend class JsonObject;
    };

    class DSINFER_CORE_EXPORT JsonArray {
    public:
        JsonArray();
        JsonArray(std::initializer_list<JsonValue> args);
        ~JsonArray();

        JsonArray(const JsonArray &other);
        JsonArray &operator=(const JsonArray &other);
        JsonArray(JsonArray &&other) noexcept;
        JsonArray &operator=(JsonArray &&other) noexcept {
            swap(other);
            return *this;
        }
        void swap(JsonArray &other) noexcept {
            std::swap(_data, other._data);
        }

    public:
        inline int size() const {
            return _data.size;
        }
        inline int count() const {
            return size();
        }

        inline bool isEmpty() const {
            return _data.size == 0;
        }
        JsonValue at(int i) const;
        inline JsonValue first() const {
            return at(0);
        }
        inline JsonValue last() const {
            return at(_data.size - 1);
        }
        inline void prepend(const JsonValue &value) {
            insert(0, value);
        }
        inline void append(const JsonValue &value) {
            insert(_data.size, value);
        }
        void removeAt(int i);
        JsonValue takeAt(int i);
        inline void removeFirst() {
            removeAt(0);
        }
        inline void removeLast() {
            removeAt(size() - 1);
        }

        void insert(int i, const JsonValue &value);
        void replace(int i, const JsonValue &value);

        bool contains(const JsonValue &element) const;
        JsonValueRef operator[](int i);
        JsonValue operator[](int i) const;

        bool operator==(const JsonArray &other) const;
        inline bool operator!=(const JsonArray &other) const {
            return !(*this == other);
        }

    public:
        class const_iterator;

        class iterator {
            JsonValueRef item;
            friend class JsonArray;

        public:
            typedef std::random_access_iterator_tag iterator_category;
            typedef int difference_type;
            typedef JsonValue value_type;
            typedef JsonValueRef reference;
            typedef JsonValueRef *pointer;

            inline iterator() : iterator(static_cast<JsonArray *>(nullptr), 0) {
            }
            explicit inline iterator(JsonArray *array, int index) : item(&array->_data, index) {
            }

            iterator(const iterator &other) = default;
            iterator &operator=(const iterator &other) = default;
            inline JsonValueRef operator*() const {
                return item;
            }
            inline const JsonValueConstRef *operator->() const {
                return &item;
            }
            inline JsonValueRef *operator->() {
                return &item;
            }
            inline JsonValueRef operator[](int j) const {
                return *(*this + j);
            }

            inline bool operator==(const iterator &o) const {
                return item.equal(o.item);
            }
            inline bool operator!=(const iterator &o) const {
                return !(*this == o);
            }
            inline bool operator<(const iterator &other) const {
                return item.idx < other.item.idx;
            }
            inline bool operator<=(const iterator &other) const {
                return item.idx <= other.item.idx;
            }
            inline bool operator>(const iterator &other) const {
                return !(*this <= other);
            }
            inline bool operator>=(const iterator &other) const {
                return !(*this < other);
            }
            inline bool operator==(const const_iterator &o) const {
                return item.arr == o.item.arr && item.idx == o.item.idx;
            }
            inline bool operator!=(const const_iterator &o) const {
                return !(*this == o);
            }
            inline bool operator<(const const_iterator &other) const {
                return item.idx < other.item.idx;
            }
            inline bool operator<=(const const_iterator &other) const {
                return item.idx <= other.item.idx;
            }
            inline bool operator>(const const_iterator &other) const {
                return !(*this <= other);
            }
            inline bool operator>=(const const_iterator &other) const {
                return !(*this < other);
            }
            inline iterator &operator++() {
                ++item.idx;
                return *this;
            }
            inline iterator operator++(int) {
                iterator n = *this;
                ++item.idx;
                return n;
            }
            inline iterator &operator--() {
                item.idx--;
                return *this;
            }
            inline iterator operator--(int) {
                iterator n = *this;
                item.idx--;
                return n;
            }
            inline iterator &operator+=(int j) {
                item.idx += j;
                return *this;
            }
            inline iterator &operator-=(int j) {
                item.idx -= j;
                return *this;
            }
            inline iterator operator+(int j) const {
                iterator r = *this;
                return r += j;
            }
            inline iterator operator-(int j) const {
                return operator+(-j);
            }
            inline int operator-(iterator j) const {
                return item.idx - j.item.idx;
            }
        };
        friend class iterator;

        class const_iterator {
            JsonValueConstRef item;
            friend class JsonArray;

        public:
            typedef std::random_access_iterator_tag iterator_category;
            typedef int difference_type;
            typedef JsonValue value_type;
            typedef const JsonValueRef reference;
            typedef const JsonValueRef *pointer;

            inline const_iterator() : const_iterator(static_cast<const JsonArray *>(nullptr), 0) {
            }
            explicit inline const_iterator(const JsonArray *array, int index)
                : item(const_cast<jsonapi_array *>(&array->_data), index) {
            }
            inline const_iterator(const iterator &o) : item(o.item) {
            }

            const_iterator(const const_iterator &other) = default;
            const_iterator &operator=(const const_iterator &other) {
                item.rebind(other.item);
                return *this;
            }
            inline const JsonValueConstRef operator*() const {
                return item;
            }
            inline const JsonValueConstRef *operator->() const {
                return &item;
            }

            inline JsonValueConstRef operator[](int j) const {
                return *(*this + j);
            }
            inline bool operator==(const const_iterator &o) const {
                return item.equal(o.item);
            }
            inline bool operator!=(const const_iterator &o) const {
                return !(*this == o);
            }
            inline bool operator<(const const_iterator &other) const {
                return item.idx < other.item.idx;
            }
            inline bool operator<=(const const_iterator &other) const {
                return item.idx <= other.item.idx;
            }
            inline bool operator>(const const_iterator &other) const {
                return !(*this <= other);
            }
            inline bool operator>=(const const_iterator &other) const {
                return !(*this < other);
            }
            inline const_iterator &operator++() {
                ++item.idx;
                return *this;
            }
            inline const_iterator operator++(int) {
                const_iterator n = *this;
                ++item.idx;
                return n;
            }
            inline const_iterator &operator--() {
                item.idx--;
                return *this;
            }
            inline const_iterator operator--(int) {
                const_iterator n = *this;
                item.idx--;
                return n;
            }
            inline const_iterator &operator+=(int j) {
                item.idx += j;
                return *this;
            }
            inline const_iterator &operator-=(int j) {
                item.idx -= j;
                return *this;
            }
            inline const_iterator operator+(int j) const {
                const_iterator r = *this;
                return r += j;
            }
            inline const_iterator operator-(int j) const {
                return operator+(-j);
            }
            inline int operator-(const_iterator j) const {
                return item.idx - j.item.idx;
            }
        };
        friend class const_iterator;

        // stl style
        inline iterator begin() {
            return iterator(this, 0);
        }
        inline const_iterator begin() const {
            return const_iterator(this, 0);
        }
        inline const_iterator constBegin() const {
            return const_iterator(this, 0);
        }
        inline const_iterator cbegin() const {
            return const_iterator(this, 0);
        }
        inline iterator end() {
            return iterator(this, size());
        }
        inline const_iterator end() const {
            return const_iterator(this, size());
        }
        inline const_iterator constEnd() const {
            return const_iterator(this, size());
        }
        inline const_iterator cend() const {
            return const_iterator(this, size());
        }
        iterator insert(iterator before, const JsonValue &value) {
            insert(before.item.idx, value);
            return before;
        }
        iterator erase(iterator it) {
            removeAt(it.item.idx);
            return it;
        }

        // more Qt
        typedef iterator Iterator;
        typedef const_iterator ConstIterator;

        // convenience
        inline JsonArray operator+(const JsonValue &v) const {
            JsonArray n = *this;
            n += v;
            return n;
        }
        inline JsonArray &operator+=(const JsonValue &v) {
            append(v);
            return *this;
        }
        inline JsonArray &operator<<(const JsonValue &v) {
            append(v);
            return *this;
        }

        // stl compatibility
        inline void push_back(const JsonValue &t) {
            append(t);
        }
        inline void push_front(const JsonValue &t) {
            prepend(t);
        }
        inline void pop_front() {
            removeFirst();
        }
        inline void pop_back() {
            removeLast();
        }
        inline bool empty() const {
            return isEmpty();
        }
        typedef int size_type;
        typedef JsonValue value_type;
        typedef value_type *pointer;
        typedef const value_type *const_pointer;
        typedef JsonValueRef reference;
        typedef JsonValue const_reference;
        typedef int difference_type;

    protected:
        JsonArray(jsonapi_array &&data);
        jsonapi_array _data;

        friend class JsonValue;
        friend class JsonValueConstRef;
        friend class JsonValueRef;
        friend class JsonDocument;
    };

    class DSINFER_CORE_EXPORT JsonObject {
    public:
        JsonObject();
        JsonObject(std::initializer_list<std::pair<std::string, JsonValue>> args);
        ~JsonObject();

        JsonObject(const JsonObject &other) noexcept;
        JsonObject &operator=(const JsonObject &other) noexcept;
        JsonObject(JsonObject &&other) noexcept;
        JsonObject &operator=(JsonObject &&other) noexcept {
            swap(other);
            return *this;
        }
        void swap(JsonObject &other) noexcept {
            std::swap(_data, other._data);
        }

    public:
        std::vector<std::string> keys() const;
        inline int size() const {
            return _data.size;
        }
        inline int count() const {
            return size();
        }
        inline int length() const {
            return size();
        }
        inline bool isEmpty() const {
            return _data.size == 0;
        }
        inline JsonValue value(const std::string &key) const {
            return value(key.data());
        }
        inline JsonValue operator[](const std::string &key) const {
            return operator[](key.data());
        }
        inline JsonValueRef operator[](const std::string &key) {
            return operator[](key.data());
        }
        inline JsonValue value(std::string_view key) const {
            return value(key.data());
        }
        JsonValue value(const char *key) const;
        inline JsonValue operator[](std::string_view key) const {
            return operator[](key.data());
        }
        JsonValue operator[](const char *key) const {
            return value(key);
        }
        inline JsonValueRef operator[](std::string_view key) {
            return operator[](key.data());
        }
        JsonValueRef operator[](const char *key);

        inline void remove(const std::string &key) {
            return remove(key.data());
        }
        inline JsonValue take(const std::string &key) {
            return take(key.data());
        }
        inline bool contains(const std::string &key) const {
            return contains(key.data());
        }
        inline void remove(std::string_view key) {
            return remove(key.data());
        }
        void remove(const char *key);
        inline JsonValue take(std::string_view key) {
            return take(key.data());
        }
        JsonValue take(const char *key);
        inline bool contains(std::string_view key) const {
            return contains(key.data());
        }
        inline bool contains(const char *key) const {
            return find(key) != end();
        }
        bool operator==(const JsonObject &other) const;
        inline bool operator!=(const JsonObject &other) const {
            return !(*this == other);
        }

    public:
        class const_iterator;

        class iterator {
            friend class const_iterator;
            friend class JsonObject;
            JsonValueRef item;

            iterator(jsonapi_object_iterator &&it)
                : item(std::forward<jsonapi_object_iterator>(it)) {
            }

        public:
            typedef std::bidirectional_iterator_tag iterator_category;
            typedef int difference_type;
            typedef JsonValue value_type;
            typedef JsonValueRef reference;
            typedef JsonValueRef *pointer;

            inline iterator() : item(jsonapi_object_iterator{nullptr, nullptr}) {
            }

            iterator(const iterator &other) = default;
            iterator &operator=(const iterator &other) {
                item.rebind(other.item);
                return *this;
            }

            inline std::string key() const {
                return item.objectKey();
            }
            inline JsonValueRef value() const {
                return item;
            }
            inline JsonValueRef operator*() const {
                return item;
            }
            inline const JsonValueConstRef *operator->() const {
                return &item;
            }
            inline JsonValueRef *operator->() {
                return &item;
            }
            inline bool operator==(const iterator &other) const {
                return item.equal(other.item);
            }
            inline bool operator!=(const iterator &other) const {
                return !(*this == other);
            }
            inline iterator &operator++() {
                item.objectIteratorNext();
                return *this;
            }
            inline iterator operator++(int) {
                iterator r = *this;
                item.objectIteratorNext();
                return r;
            }
            inline iterator &operator--() {
                item.objectIteratorPrev();
                return *this;
            }
            inline iterator operator--(int) {
                iterator r = *this;
                item.objectIteratorPrev();
                return r;
            }

            inline bool operator==(const const_iterator &other) const {
                return item.equal(other.item);
            }
            inline bool operator!=(const const_iterator &other) const {
                return !(*this == other);
            }
        };
        friend class iterator;

        class const_iterator {
            friend class iterator;
            JsonValueConstRef item;

            const_iterator(jsonapi_object_iterator &&it)
                : item(std::forward<jsonapi_object_iterator>(it)) {
            }

        public:
            typedef std::bidirectional_iterator_tag iterator_category;
            typedef int difference_type;
            typedef JsonValue value_type;
            typedef const JsonValueConstRef reference;
            typedef const JsonValueConstRef *pointer;

            inline const_iterator() : item(jsonapi_object_iterator{nullptr, nullptr}) {
            }
            inline const_iterator(const iterator &other) : item(other.item) {
            }
            const_iterator(const const_iterator &other) = default;
            const_iterator &operator=(const const_iterator &other) {
                item.rebind(other.item);
                return *this;
            }
            inline std::string key() const {
                return item.objectKey();
            }
            inline JsonValueConstRef value() const {
                return item;
            }
            inline const JsonValueConstRef operator*() const {
                return item;
            }
            inline const JsonValueConstRef *operator->() const {
                return &item;
            }
            inline bool operator==(const const_iterator &other) const {
                return item.equal(other.item);
            }
            inline bool operator!=(const const_iterator &other) const {
                return !(*this == other);
            }
            inline const_iterator &operator++() {
                item.objectIteratorNext();
                return *this;
            }
            inline const_iterator operator++(int) {
                const_iterator r = *this;
                item.objectIteratorNext();
                return r;
            }
            inline const_iterator &operator--() {
                item.objectIteratorPrev();
                return *this;
            }
            inline const_iterator operator--(int) {
                const_iterator r = *this;
                item.objectIteratorPrev();
                return r;
            }
            inline bool operator==(const iterator &other) const {
                return item.equal(other.item);
            }
            inline bool operator!=(const iterator &other) const {
                return !(*this == other);
            }
        };
        friend class const_iterator;

        // STL style
        iterator begin();
        const_iterator begin() const {
            return constBegin();
        }
        const_iterator constBegin() const;
        iterator end();
        inline const_iterator end() const {
            return constEnd();
        }
        const_iterator constEnd() const;
        iterator erase(iterator it);

        // more Qt
        typedef iterator Iterator;
        typedef const_iterator ConstIterator;
        iterator find(const std::string &key) {
            return find(key.data());
        }
        const_iterator find(const std::string &key) const {
            return find(key.data());
        }
        const_iterator constFind(const std::string &key) const {
            return constFind(key.data());
        }
        iterator insert(const std::string &key, const JsonValue &value) {
            return insert(key.data(), value);
        }
        iterator find(std::string_view key) {
            return find(key.data());
        }
        iterator find(const char *key);
        inline const_iterator find(std::string_view key) const {
            return constFind(key);
        }
        inline const_iterator find(const char *key) const {
            return constFind(key);
        }
        inline const_iterator constFind(std::string_view key) const {
            return constFind(key.data());
        }
        const_iterator constFind(const char *key) const;
        inline iterator insert(std::string_view key, const JsonValue &value) {
            return insert(key.data(), value);
        }
        iterator insert(const char *key, const JsonValue &value);

        // STL compatibility
        typedef JsonValue mapped_type;
        typedef std::string key_type;
        typedef int size_type;

        inline bool empty() const {
            return isEmpty();
        }

    protected:
        JsonObject(jsonapi_object &&data);
        jsonapi_object _data;

        friend class JsonValue;
        friend class JsonValueConstRef;
        friend class JsonValueRef;
        friend class JsonDocument;
    };

    class DSINFER_CORE_EXPORT JsonDocument {
    public:
        JsonDocument();
        explicit JsonDocument(const JsonObject &object);
        explicit JsonDocument(const JsonArray &array);
        ~JsonDocument();

        JsonDocument(const JsonDocument &other);
        JsonDocument &operator=(const JsonDocument &other);
        JsonDocument(JsonDocument &&other) noexcept;
        JsonDocument &operator=(JsonDocument &&other) noexcept {
            swap(other);
            return *this;
        }
        inline void swap(JsonDocument &other) noexcept {
            std::swap(_data, other._data);
        }

    public:
        static JsonDocument fromJson(const char *json, std::string *error = nullptr);
        std::string toJson(int options = 0) const;

        bool isEmpty() const;
        bool isArray() const;
        bool isObject() const;
        bool isNull() const;

        JsonObject object() const;
        JsonArray array() const;

        void setObject(const JsonObject &object);
        void setArray(const JsonArray &array);

    private:
        JsonDocument(jsonapi_value &&data);
        jsonapi_value _data;
    };

}

#endif // JSONAPI_H
