#ifndef JSONAPI_CAPI_H
#define JSONAPI_CAPI_H

#include <dsinferCore/dsinfercoreglobal.h>

#ifdef __cplusplus
extern "C" {
#endif

enum jsonapi_type {
    jsonapi_Type_Null = 0x0,       // -> nullptr
    jsonapi_Type_Bool = 0x1,       // -> int
    jsonapi_Type_Double = 0x2,     // -> double
    jsonapi_Type_String = 0x3,     // -> jsonapi_string
    jsonapi_Type_Array = 0x4,      // -> jsonapi_array
    jsonapi_Type_Object = 0x5,     // -> jsonapi_object
    jsonapi_Type_Undefined = 0x80, // -> nullptr
};

struct jsonapi_string {
    int size;
    char *data;
    int managed;
};

DSINFER_CORE_EXPORT void jsonapi_string_create(jsonapi_string *s, const char *data,
                                               int managed = false);
DSINFER_CORE_EXPORT void jsonapi_string_destroy(jsonapi_string *s);

struct jsonapi_array {
    void *data;
    int size;
};

struct jsonapi_object {
    void *data;
    int size;
};

struct jsonapi_object_iterator {
    jsonapi_object *obj;
    void *data;
};

struct jsonapi_value {
    void *data;
    int type;
};

struct jsonapi_bridge {
    void (*array_create)(struct jsonapi_array *a);
    void (*array_destroy)(struct jsonapi_array *a);
    void (*array_get)(const struct jsonapi_array *a, int i, struct jsonapi_value *v);
    void (*array_replace)(struct jsonapi_array *a, int i, const struct jsonapi_value *v);
    void (*array_insert)(struct jsonapi_array *a, int i, const struct jsonapi_value *v);
    void (*array_remove)(struct jsonapi_array *a, int i);
    void (*array_copy)(const struct jsonapi_array *src, struct jsonapi_array *dst);
    bool (*array_equal)(const struct jsonapi_array *a1, const struct jsonapi_array *a2);

    void (*object_create)(struct jsonapi_object *o);
    void (*object_destroy)(struct jsonapi_object *o);
    void (*object_copy)(const struct jsonapi_object *src, struct jsonapi_object *dst);
    bool (*object_equal)(const struct jsonapi_object *a1, const struct jsonapi_object *a2);

    void (*object_find)(const jsonapi_object *o, const char *k, struct jsonapi_object_iterator *i);
    void (*object_insert)(struct jsonapi_object *o, const char *k, const struct jsonapi_value *v,
                          struct jsonapi_object_iterator *i);
    void (*object_erase)(struct jsonapi_object *o, struct jsonapi_object_iterator *i);
    void (*object_iterator_destroy)(struct jsonapi_object_iterator *i);
    bool (*object_iterator_equal)(const struct jsonapi_object_iterator *i1,
                                  const struct jsonapi_object_iterator *i2);
    void (*object_iterator_copy)(const struct jsonapi_object_iterator *src,
                                 struct jsonapi_object_iterator *dst);

    void (*object_begin)(const jsonapi_object *o, struct jsonapi_object_iterator *i);
    void (*object_end)(const jsonapi_object *o, struct jsonapi_object_iterator *i);
    void (*object_iterator_next)(struct jsonapi_object_iterator *i);
    void (*object_iterator_prev)(struct jsonapi_object_iterator *i);
    void (*object_iterator_key)(const struct jsonapi_object_iterator *i, struct jsonapi_string *v);
    void (*object_iterator_get)(const struct jsonapi_object_iterator *i, struct jsonapi_value *v);
    void (*object_iterator_set)(struct jsonapi_object_iterator *i, const struct jsonapi_value *v);

    void (*value_create)(struct jsonapi_value *v, int type, const void *data);
    void (*value_destroy)(struct jsonapi_value *v);
    void (*value_get)(const struct jsonapi_value *v, void *data);
    void (*value_copy)(const struct jsonapi_value *src, struct jsonapi_value *dst);
    bool (*value_equal)(const struct jsonapi_value *a1, const struct jsonapi_value *a2);

    void (*document_serialize)(const struct jsonapi_value *v, jsonapi_string *buf, int options);
    void (*document_deserialize)(struct jsonapi_value *v, jsonapi_string *err);
};

DSINFER_CORE_EXPORT void jsonapi_bridge_global_set(const jsonapi_bridge *b);
DSINFER_CORE_EXPORT void jsonapi_bridge_global_get(jsonapi_bridge *b);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // JSONAPI_CAPI_H
