/* test_types.h - Comprehensive type definitions for gengtype coverage */

#ifndef TEST_TYPES_H
#define TEST_TYPES_H

#include <stddef.h>

/* TYPE_SCALAR: Basic scalar type definitions */
typedef int my_scalar;
typedef long my_long_scalar;
typedef unsigned char my_uchar_scalar;

/* TYPE_UNDEFINED: Forward declarations that might remain undefined */
struct undefined_struct;
typedef struct undefined_struct *undefined_ptr_t;

/* TYPE_STRING: String type with length annotation */
struct string_container {
    char * GTY((length("str_len"))) data;
    size_t str_len;
};

/* TYPE_STRUCT: Plain C structs with GTY tags */
struct GTY(()) plain_struct {
    int field1;
    double field2;
    my_scalar field3;
};

/* TYPE_USER_STRUCT: User-defined struct with custom marker */
struct GTY((user)) user_marked_struct {
    int user_data;
    void *user_ptr;
};

/* TYPE_UNION: Union containing GTY-tagged fields */
union GTY(()) tagged_union {
    int int_val;
    double double_val;
    struct plain_struct * GTY((tag("0"))) struct_ptr;
    char * GTY((tag("1"))) string_ptr;
};

/* TYPE_POINTER: Struct containing various pointers */
struct GTY(()) pointer_container {
    struct plain_struct * GTY((skip)) ptr1;
    union tagged_union *ptr2;
    struct string_container * GTY((reorder)) ptr3;
    int *int_ptr;
    void *opaque_ptr;
};

/* TYPE_ARRAY: Various array types */
struct GTY(()) array_container {
    int fixed_array[10];
    struct plain_struct * GTY((length("var_len"))) variable_array[];
    size_t var_len;
};

/* TYPE_CALLBACK: Function pointer/callback types */
typedef void (* GTY((callback)) callback_func)(int, void*);
typedef int (*comparator_func)(const void *, const void *);

struct GTY(()) callback_container {
    callback_func cb;
    comparator_func cmp;
    void * GTY((skip)) user_data;
};

/* TYPE_LANG_STRUCT: Language-specific struct with hooks */
struct GTY((lang_struct("C"))) language_specific {
    int lang_specific_field;
    void *lang_ptr;
};

/* Complex nested type to ensure deep traversal */
struct GTY(()) complex_nested {
    struct pointer_container *pc;
    union tagged_union tu;
    struct array_container ac;
    struct GTY((desc("%1.type"))) variant {
        int type;
        union {
            int int_val;
            double double_val;
            struct string_container *str;
        } GTY((tag("%0.type"))) data;
    } var;
};

/* Another user struct for good measure */
struct GTY((user)) another_user_struct {
    struct user_marked_struct *ums;
    callback_func handler;
};

/* Template-like macro to generate more types */
#define DECLARE_TYPED_CONTAINER(name, type) \
    struct GTY(()) name##_container { \
        type * GTY((skip)) items; \
        size_t count; \
        size_t capacity; \
    }

DECLARE_TYPED_CONTAINER(int, int);
DECLARE_TYPED_CONTAINER(string, struct string_container);

/* Opaque pointer type */
typedef void * GTY((skip)) opaque_handle_t;

#endif /* TEST_TYPES_H */
