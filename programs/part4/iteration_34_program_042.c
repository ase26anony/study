#ifndef TEST_TYPES_H
#define TEST_TYPES_H

#include <stddef.h>

/* TYPE_SCALAR: Basic scalar type definitions */
typedef int my_scalar;
typedef long my_long_scalar;
typedef unsigned char my_uchar_scalar;

/* TYPE_POINTER: Pointer type definitions */
typedef my_scalar *my_scalar_ptr;
typedef void *generic_ptr;

/* TYPE_CALLBACK: Function pointer/callback types */
typedef int (*comparison_fn)(const void *, const void *);
typedef void (*cleanup_fn)(void *);

/* TYPE_ARRAY: Array type (fixed size) */
typedef int my_fixed_array[10];

/* String type callback for variable length arrays */
struct string_info {
    size_t length;
    char *data;
};

/* TYPE_STRUCT: Plain C structs with GTY annotations */
struct GTY(()) simple_struct {
    int field1;
    char field2;
    float field3;
};

/* Another struct with pointer field */
struct GTY(()) struct_with_ptr {
    int id;
    struct simple_struct *GTY((skip)) next;
    char *GTY((length("strlen(%h.data) + 1"))) data;
};

/* TYPE_USER_STRUCT: User-defined struct with custom markers */
struct GTY((user)) user_defined_struct {
    int user_id;
    void *user_data;
    /* User-defined traversal function would be defined elsewhere */
};

/* TYPE_UNION: Union type */
union GTY(()) my_union {
    int as_int;
    float as_float;
    char *GTY((tag("0"))) as_string;
    struct simple_struct *GTY((tag("1"))) as_struct;
};

/* TYPE_STRING: String type with length annotation */
struct GTY(()) string_container {
    char *GTY((length("strlen(%h.data)"))) data;
    size_t length;
};

/* Array with variable length */
struct GTY(()) var_array_struct {
    int count;
    int GTY((length("%h.count"))) items[];
};

/* Nested struct for complex type graph */
struct GTY(()) outer_struct {
    struct simple_struct inner;
    struct struct_with_ptr *ptr_field;
    union my_union union_field;
    my_fixed_array array_field;
};

/* TYPE_LANG_STRUCT: Language-specific struct */
struct GTY((lang_struct)) lang_specific {
    int lang_id;
    void *lang_data;
    /* Language-specific hooks would be defined elsewhere */
};

/* Callback struct with function pointers */
struct GTY(()) callback_container {
    comparison_fn compare;
    cleanup_fn cleanup;
    void *GTY((skip)) user_data;
};

/* Complex type with multiple pointer levels */
struct GTY(()) complex_type {
    struct outer_struct **GTY((refferal("outer_struct"))) refs;
    struct callback_container *callbacks;
    struct var_array_struct *var_array;
};

/* Forward declarations for circular references */
struct GTY(()) node_a;
struct GTY(()) node_b;

struct GTY(()) node_a {
    int id;
    struct node_b *GTY((skip)) partner;
};

struct GTY(()) node_b {
    int id;
    struct node_a *GTY((skip)) partner;
};

/* Template-like macro for generating multiple struct types */
#define DECLARE_STRUCT_TYPE(name, field_type) \
    struct GTY(()) name { \
        int id; \
        field_type value; \
    }

/* Generate several struct types using the macro */
DECLARE_STRUCT_TYPE(int_struct, int);
DECLARE_STRUCT_TYPE(ptr_struct, void*);
DECLARE_STRUCT_TYPE(array_struct, int[5]);

/* Anonymous struct/union */
struct GTY(()) container {
    struct {
        int x;
        int y;
    } point;
    union {
        int i;
        float f;
    } value;
};

/* Struct with bitfields */
struct GTY(()) bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 2;
    unsigned int flag3 : 3;
    int regular_field;
};

/* Struct with __attribute__ extensions */
struct GTY(()) attributed_struct {
    int field __attribute__((aligned(16)));
    char *data __attribute__((nonstring));
} __attribute__((packed));

#endif /* TEST_TYPES_H */
