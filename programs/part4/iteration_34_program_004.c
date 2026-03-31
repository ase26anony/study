/* test_types.h - Comprehensive type definitions for gengtype coverage */

#ifndef TEST_TYPES_H
#define TEST_TYPES_H

#include <stddef.h>

/* TYPE_SCALAR: Basic scalar type */
typedef int my_scalar GTY(());

/* TYPE_STRING: String type with length callback */
struct GTY(()) string_struct {
    char* GTY((length("str_len"))) data;
    size_t str_len;
};

/* TYPE_STRUCT: Plain C struct */
struct GTY(()) plain_struct {
    int field1;
    double field2;
    my_scalar scalar_field;
};

/* TYPE_USER_STRUCT: User-defined struct with custom markers */
struct GTY((user)) user_struct {
    void* opaque_data;
    int user_id;
};

/* TYPE_UNION: Union type */
union GTY(()) tagged_union {
    int int_val;
    double double_val;
    char* GTY((tag("0"))) string_val;
    struct plain_struct* GTY((tag("1"))) struct_ptr;
};

/* TYPE_POINTER: Various pointer types */
struct GTY(()) pointer_container {
    struct plain_struct* GTY(()) struct_ptr;
    union tagged_union* GTY(()) union_ptr;
    my_scalar* GTY(()) scalar_ptr;
    void (*GTY(()) func_ptr)(void);
};

/* TYPE_ARRAY: Array types (fixed and variable length) */
struct GTY(()) array_container {
    int fixed_array[10];
    int* GTY((length("dynamic_len"))) dynamic_array;
    size_t dynamic_len;
    struct plain_struct struct_array[5];
};

/* TYPE_CALLBACK: Callback function type */
typedef void (*GTY((callback)) callback_func)(struct plain_struct*, int);

struct GTY(()) callback_container {
    callback_func handler;
    void* GTY((skip)) user_data;
};

/* TYPE_LANG_STRUCT: Language-specific struct */
struct GTY((lang_struct)) lang_specific {
    int lang_id;
    void* lang_data;
};

/* Complex nested type to ensure deep traversal */
struct GTY(()) complex_nested {
    struct pointer_container* GTY(()) ptrs;
    struct array_container arrays;
    union tagged_union variant;
    struct GTY(()) inner_struct {
        int depth;
        struct complex_nested* GTY(()) parent;
    } inner;
};

/* Forward declarations for circular references */
struct GTY(()) forward_decl;
typedef struct forward_decl* forward_ptr;

struct GTY(()) forward_decl {
    int id;
    forward_ptr GTY((skip)) next;  /* Skip to avoid infinite recursion */
    struct complex_nested* GTY(()) complex_ref;
};

/* TYPE_UNDEFINED: Incomplete/undefined type */
struct GTY(()) undefined_container {
    struct incomplete* GTY(()) undefined_ptr;  /* Forward declared but never defined */
};

/* Function pointer array */
typedef void (*GTY((callback)) func_array[5])(void);

/* Template-like macro for generating multiple struct types */
#define DECLARE_STRUCT_TYPE(name, field_type) \
    struct GTY(()) name##_struct { \
        field_type GTY(()) value; \
        int id; \
    }

DECLARE_STRUCT_TYPE(int_wrapper, int);
DECLARE_STRUCT_TYPE(double_wrapper, double);
DECLARE_STRUCT_TYPE(ptr_wrapper, void*);

/* Enumeration type */
typedef enum GTY(()) {
    MODE_A,
    MODE_B,
    MODE_C
} operation_mode;

struct GTY(()) mode_container {
    operation_mode current_mode;
    int mode_data;
};

/* Bitfield struct */
struct GTY(()) bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 2;
    unsigned int flag3 : 3;
    unsigned int padding : 26;
};

/* Variable length struct at end */
struct GTY(()) var_len_struct {
    int count;
    int data[1];  /* Variable length array idiom */
};

#endif /* TEST_TYPES_H */
