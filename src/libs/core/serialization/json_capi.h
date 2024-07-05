#ifndef DSINFERCORE_JSON_CAPI_H
#define DSINFERCORE_JSON_CAPI_H

#include <dsinferCore/dsinfercoreglobal.h>

#ifdef __cplusplus
extern "C" {
#endif

enum dsinfer_json_type {
    dsinfer_json_Type_Null = 0x0,       // -> nullptr
    dsinfer_json_Type_Bool = 0x1,       // -> int
    dsinfer_json_Type_Double = 0x2,     // -> double
    dsinfer_json_Type_String = 0x3,     // -> dsinfer_json_string
    dsinfer_json_Type_Array = 0x4,      // -> dsinfer_json_array
    dsinfer_json_Type_Object = 0x5,     // -> dsinfer_json_object
    dsinfer_json_Type_Undefined = 0x80, // -> nullptr
};

struct dsinfer_json_string {
    int size;
    char *data;
    int managed;
};

DSINFER_CORE_EXPORT void dsinfer_json_string_create(dsinfer_json_string *s, const char *data,
                                               int managed = false);
DSINFER_CORE_EXPORT void dsinfer_json_string_destroy(dsinfer_json_string *s);

struct dsinfer_json_array {
    void *data;
    int size;
};

struct dsinfer_json_object {
    void *data;
    int size;
};

struct dsinfer_json_object_iterator {
    dsinfer_json_object *obj;
    void *data;
};

struct dsinfer_json_value {
    void *data;
    int type;
};

struct dsinfer_json_bridge {
    void (*array_create)(struct dsinfer_json_array *a);
    void (*array_destroy)(struct dsinfer_json_array *a);
    void (*array_get)(const struct dsinfer_json_array *a, int i, struct dsinfer_json_value *v);
    void (*array_replace)(struct dsinfer_json_array *a, int i, const struct dsinfer_json_value *v);
    void (*array_insert)(struct dsinfer_json_array *a, int i, const struct dsinfer_json_value *v);
    void (*array_remove)(struct dsinfer_json_array *a, int i);
    void (*array_copy)(const struct dsinfer_json_array *src, struct dsinfer_json_array *dst);
    bool (*array_equal)(const struct dsinfer_json_array *a1, const struct dsinfer_json_array *a2);

    void (*object_create)(struct dsinfer_json_object *o);
    void (*object_destroy)(struct dsinfer_json_object *o);
    void (*object_copy)(const struct dsinfer_json_object *src, struct dsinfer_json_object *dst);
    bool (*object_equal)(const struct dsinfer_json_object *a1, const struct dsinfer_json_object *a2);

    void (*object_find)(const dsinfer_json_object *o, const char *k, struct dsinfer_json_object_iterator *i);
    void (*object_insert)(struct dsinfer_json_object *o, const char *k, const struct dsinfer_json_value *v,
                          struct dsinfer_json_object_iterator *i);
    void (*object_erase)(struct dsinfer_json_object *o, struct dsinfer_json_object_iterator *i);
    void (*object_iterator_destroy)(struct dsinfer_json_object_iterator *i);
    bool (*object_iterator_equal)(const struct dsinfer_json_object_iterator *i1,
                                  const struct dsinfer_json_object_iterator *i2);
    void (*object_iterator_copy)(const struct dsinfer_json_object_iterator *src,
                                 struct dsinfer_json_object_iterator *dst);

    void (*object_begin)(const dsinfer_json_object *o, struct dsinfer_json_object_iterator *i);
    void (*object_end)(const dsinfer_json_object *o, struct dsinfer_json_object_iterator *i);
    void (*object_iterator_next)(struct dsinfer_json_object_iterator *i);
    void (*object_iterator_prev)(struct dsinfer_json_object_iterator *i);
    void (*object_iterator_key)(const struct dsinfer_json_object_iterator *i, struct dsinfer_json_string *v);
    void (*object_iterator_get)(const struct dsinfer_json_object_iterator *i, struct dsinfer_json_value *v);
    void (*object_iterator_set)(struct dsinfer_json_object_iterator *i, const struct dsinfer_json_value *v);

    void (*value_create)(struct dsinfer_json_value *v, int type, const void *data);
    void (*value_destroy)(struct dsinfer_json_value *v);
    void (*value_get)(const struct dsinfer_json_value *v, void *data);
    void (*value_copy)(const struct dsinfer_json_value *src, struct dsinfer_json_value *dst);
    bool (*value_equal)(const struct dsinfer_json_value *a1, const struct dsinfer_json_value *a2);

    void (*json_serialize)(const struct dsinfer_json_value *v, dsinfer_json_string *buf, int options);
    void (*json_deserialize)(struct dsinfer_json_value *v, dsinfer_json_string *err);
    void (*cbor_serialize)(const struct dsinfer_json_value *v, dsinfer_json_string *buf, int options);
    void (*cbor_deserialize)(struct dsinfer_json_value *v, dsinfer_json_string *err);
};

DSINFER_CORE_EXPORT void dsinfer_json_bridge_global_set(const dsinfer_json_bridge *b);
DSINFER_CORE_EXPORT void dsinfer_json_bridge_global_get(dsinfer_json_bridge *b);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // DSINFERCORE_JSON_CAPI_H
