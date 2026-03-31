#ifndef TEST_TYPES_H
#define TEST_TYPES_H

#include <stddef.h>

/* TYPE_SCALAR: Basic scalar typedef */
typedef int my_scalar GTY(());
typedef unsigned long my_other_scalar GTY(());

/* TYPE_STRING: String type with length annotation */
struct string_struct GTY(()) {
    char* data GTY((length("str_len")));
    int str_len;
};

/* TYPE_STRUCT: Plain struct */
struct plain_struct GTY(()) {
    int x;
    double y;
    my_scalar z;
};

/* TYPE_USER_STRUCT: User-defined struct */
struct user_defined GTY((user)) {
    void* custom_data;
    int (*user_func)(void*);
};

/* TYPE_UNION: Union type */
union my_union GTY(()) {
    int as_int;
    double as_double;
    void* as_ptr;
    struct plain_struct as_struct;
};

/* TYPE_POINTER: Pointer types */
struct pointer_struct GTY(()) {
    struct plain_struct* next GTY((skip));
    struct string_struct* str_ptr;
    union my_union* union_ptr;
    void* opaque_ptr GTY((skip));
};

/* TYPE_ARRAY: Array types */
struct array_struct GTY(()) {
    int fixed_array[10];
    int* variable_array GTY((length("var_len")));
    size_t var_len;
    struct plain_struct struct_array[5];
};

/* TYPE_CALLBACK: Callback/function pointer type */
typedef int (*callback_type)(int, void*) GTY((callback));

struct callback_struct GTY(()) {
    callback_type handler;
    void* user_data GTY((skip));
};

/* TYPE_LANG_STRUCT: Language-specific struct */
struct lang_specific GTY((lang_struct)) {
    int lang_field1;
    void* lang_field2 GTY((skip));
    struct {
        int nested;
        char* name GTY((length("name_len")));
        size_t name_len;
    } inner;
};

/* Complex nested type to ensure deep traversal */
struct container GTY(()) {
    struct plain_struct base;
    struct string_struct* strings GTY((length("string_count")));
    struct array_struct arrays[3];
    union myUnion* optional_data;
    callback_type callbacks[2];
    size_t string_count;
};

/* Forward declarations for pointer cycles */
struct forward_decl GTY(());
struct another_forward GTY(());

struct forward_decl GTY(()) {
    int id;
    struct another_forward* link;
};

struct another_forward GTY(()) {
    char* name GTY((length("name_len")));
    struct forward_decl* backlink;
    size_t name_len;
};

/* TYPE_UNDEFINED: This might be triggered by incomplete types */
struct incomplete;  /* Forward declaration without definition */

/* Enum type (also scalar) */
typedef enum {
    VALUE_A,
    VALUE_B,
    VALUE_C
} my_enum GTY(());

/* Bitfield struct */
struct bitfield_struct GTY(()) {
    unsigned int flag1 : 1;
    unsigned int flag2 : 2;
    unsigned int flag3 : 3;
    int regular_field;
};

/* Variable length struct at end */
struct trailing_array GTY(()) {
    int count;
    int items[1] GTY((variable_length));
};

#endif /* TEST_TYPES_H */
