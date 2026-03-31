/* test_types.h - Comprehensive type definitions for gengtype coverage */

#ifndef TEST_TYPES_H
#define TEST_TYPES_H

#include <stddef.h>

/* TYPE_SCALAR: Basic scalar type */
typedef int my_scalar GTY(());

/* TYPE_STRING: String type with length annotation */
struct string_struct GTY(()) {
    char *data GTY((length("str_len")));
    size_t str_len;
};

/* TYPE_STRUCT: Plain C struct */
struct plain_struct GTY(()) {
    int x;
    double y;
    my_scalar z;
};

/* TYPE_USER_STRUCT: User-defined struct with custom marker */
struct user_defined GTY((user)) {
    void *custom_data;
    int user_tag;
};

/* TYPE_UNION: Union type */
union variant_type GTY(()) {
    int as_int;
    double as_double;
    char *as_string GTY((length("10")));
    struct plain_struct *as_struct GTY(());
};

/* TYPE_POINTER: Various pointer types */
struct pointer_container GTY(()) {
    struct plain_struct *next GTY(());
    union variant_type *variant_ptr GTY(());
    struct string_struct **double_ptr GTY((skip));
    void *opaque_ptr;
};

/* TYPE_ARRAY: Array types (fixed and variable length) */
struct array_container GTY(()) {
    int fixed_array[10];
    struct plain_struct *var_array GTY((length("array_len")));
    size_t array_len;
    char *dynamic_strings[] GTY((length("string_count")));
    int string_count;
};

/* TYPE_CALLBACK: Callback function type */
typedef void (*callback_func)(int, void*) GTY((callback));

struct callback_container GTY(()) {
    callback_func handler;
    void *user_data;
};

/* TYPE_LANG_STRUCT: Language-specific struct */
struct lang_specific GTY((lang_struct)) {
    int lang_id;
    void *lang_data;
};

/* Complex nested structure to ensure deep traversal */
struct nested_types GTY(()) {
    /* Contains one of each type kind */
    my_scalar scalar_field;
    struct string_struct string_field;
    struct plain_struct struct_field;
    struct user_defined *user_ptr GTY(());
    union variant_type union_field;
    struct pointer_container *pointer_field GTY(());
    struct array_container array_field;
    struct callback_container callback_field;
    struct lang_specific lang_field;
    
    /* Self-referential for pointer chains */
    struct nested_types *next GTY(());
    struct nested_types **prev GTY((skip));
};

/* Forward declarations for mutual recursion */
struct forward_declared GTY(());
struct mutual_a GTY(());
struct mutual_b GTY(());

struct forward_declared {
    int id;
    struct mutual_a *link_a GTY(());
};

struct mutual_a {
    int value;
    struct mutual_b *partner GTY(());
};

struct mutual_b {
    char name[20];
    struct mutual_a *owner GTY(());
};

/* Enum type (treated as scalar by gengtype) */
typedef enum {
    ENUM_VAL_A,
    ENUM_VAL_B,
    ENUM_VAL_C
} my_enum GTY(());

/* Bitfield structure */
struct bitfield_struct GTY(()) {
    unsigned int flag1 : 1;
    unsigned int flag2 : 2;
    unsigned int flag3 : 3;
    unsigned int padding : 26;
};

/* Function pointer in struct */
struct with_function_ptr GTY(()) {
    int (*compare)(const void*, const void*);
    void (*destructor)(void*);
};

/* Anonymous union within struct */
struct with_anon_union GTY(()) {
    int type;
    union {
        int int_val;
        double double_val;
        char *string_val GTY((length("20")));
    } data;
};

/* Zero-length array */
struct with_flex_array GTY(()) {
    int count;
    int items[];
};

#endif /* TEST_TYPES_H */
