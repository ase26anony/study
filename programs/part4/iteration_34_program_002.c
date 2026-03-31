/* test_types.h - Comprehensive type definitions for gengtype coverage */

#ifndef TEST_TYPES_H
#define TEST_TYPES_H

#include <stddef.h>

/* TYPE_SCALAR: Basic scalar type */
typedef int my_scalar GTY(());
typedef unsigned long another_scalar GTY(());

/* TYPE_STRING: String type with length annotation */
struct GTY(()) string_struct {
    char * GTY((length("str_len"))) data;
    int str_len;
};

/* TYPE_STRUCT: Plain C struct */
struct GTY(()) plain_struct {
    int field1;
    double field2;
    my_scalar field3;
};

/* TYPE_USER_STRUCT: User-defined struct with custom marker */
struct GTY((user)) user_defined_struct {
    int user_data;
    void *user_ptr;
};

/* TYPE_UNION: Union type */
union GTY(()) my_union {
    int int_val;
    double double_val;
    char * GTY((length("str_len"))) string_val;
    struct plain_struct *struct_ptr;
};

/* TYPE_POINTER: Pointer types */
struct GTY(()) pointer_container {
    struct plain_struct * GTY((skip)) ptr1;
    struct string_struct ** GTY((tag("0"))) ptr2;
    void * GTY((atomic)) atomic_ptr;
};

/* TYPE_ARRAY: Array types */
struct GTY(()) array_container {
    int fixed_array[10];
    int * GTY((length("dynamic_len"))) dynamic_array;
    size_t dynamic_len;
    
    /* Nested array in struct */
    struct plain_struct struct_array[5];
};

/* TYPE_CALLBACK: Callback/function pointer types */
typedef void (* GTY((callback)) callback_func)(int, void*);

struct GTY(()) callback_container {
    callback_func GTY((skip)) handler;
    void (* GTY((callback)) another_handler)(struct plain_struct*);
};

/* TYPE_LANG_STRUCT: Language-specific struct */
struct GTY((lang_struct)) lang_specific {
    int lang_data;
    void *lang_hook;
};

/* Complex nested type to ensure deep traversal */
struct GTY(()) complex_nested {
    union my_union variant;
    struct array_container arrays;
    struct pointer_container *pointers;
    callback_func on_event;
    
    /* Self-referential pointer */
    struct complex_nested * GTY((skip)) next;
};

/* Forward declarations for pointer cycles */
struct forward_declared GTY(());
struct another_forward GTY(());

struct forward_declared {
    int data;
    struct another_forward *link;
};

struct another_forward {
    char *name;
    struct forward_declared *back_link;
};

/* TYPE_UNDEFINED: This might be trickier to trigger directly,
   but we can try with incomplete types or special cases */
typedef void undefined_type GTY(());

/* Template-like macro to generate more types */
#define DECLARE_GTY_STRUCT(name, field1_type, field2_type) \
    struct GTY(()) name { \
        field1_type f1; \
        field2_type f2; \
    }

DECLARE_GTY_STRUCT(generated_struct, int, double);
DECLARE_GTY_STRUCT(another_generated, my_scalar, struct string_struct*);

/* Enumeration type (also scalar in some contexts) */
typedef enum GTY(()) my_enum {
    ENUM_VAL1,
    ENUM_VAL2,
    ENUM_VAL3
} my_enum_type;

/* Bitfield struct */
struct GTY(()) bitfield_struct {
    unsigned int flag1:1;
    unsigned int flag2:2;
    unsigned int flag3:3;
    int regular_field;
};

/* Variable length struct */
struct GTY(()) var_len_struct {
    int count;
    int items GTY((length("count"))) [];
};

#endif /* TEST_TYPES_H */
