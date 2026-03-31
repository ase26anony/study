/* test_types.h - Comprehensive type definitions for gengtype coverage */

#ifndef TEST_TYPES_H
#define TEST_TYPES_H

#include <stddef.h>

/* TYPE_SCALAR: Basic scalar type */
typedef int my_scalar GTY(());

/* TYPE_STRING: String type with length attribute */
struct GTY(()) string_struct {
    char* GTY((length("%h.length"))) data;
    size_t length;
};

/* TYPE_STRUCT: Plain C struct */
struct GTY(()) plain_struct {
    int field1;
    double field2;
    my_scalar scalar_field;
};

/* TYPE_USER_STRUCT: User-defined struct with custom markers */
struct GTY((user)) user_struct {
    void* custom_data;
    int user_id;
};

/* TYPE_UNION: Union type */
union GTY(()) tagged_union {
    int int_val;
    double double_val;
    char* GTY((tag("0"))) str_val;
    struct plain_struct* GTY((tag("1"))) struct_ptr;
};

/* TYPE_POINTER: Pointer types */
struct GTY(()) pointer_container {
    struct plain_struct* GTY(()) struct_ptr;
    my_scalar* GTY(()) scalar_ptr;
    union tagged_union* GTY(()) union_ptr;
    void (* GTY(()) func_ptr)(void);
};

/* TYPE_ARRAY: Array types */
struct GTY(()) array_container {
    int fixed_array[10];
    char* GTY((length("%h.dyn_length"))) dynamic_array;
    size_t dyn_length;
    struct plain_struct GTY(()) struct_array[5];
};

/* TYPE_CALLBACK: Callback/function pointer type */
typedef void (* GTY((callback))) callback_func(int, void*);

struct GTY(()) callback_container {
    callback_func* handler;
    void* GTY((skip)) user_data;
};

/* TYPE_LANG_STRUCT: Language-specific struct */
#ifdef __cplusplus
extern "C" {
#endif

struct GTY((lang_struct)) lang_specific {
    int lang_id;
    void* lang_data;
};

#ifdef __cplusplus
}
#endif

/* Complex nested type to ensure deep traversal */
struct GTY(()) complex_nested {
    struct pointer_container* GTY(()) ptrs;
    struct array_container arrays;
    union tagged_union variant;
    struct GTY(()) inner_struct {
        int depth;
        struct complex_nested* GTY(()) next;
    } inner;
};

/* Forward declaration for circular reference */
struct GTY(()) forward_declared;
struct GTY(()) forward_declared {
    int value;
    struct forward_declared* GTY(()) next;
};

/* TYPE_UNDEFINED: Should be caught by incomplete types */
struct GTY(()) incomplete_struct;  /* Forward declaration only */

/* Enum type (also scalar) */
typedef enum GTY(()) my_enum {
    ENUM_VAL1,
    ENUM_VAL2,
    ENUM_VAL3
} my_enum_type;

/* Bitfield struct */
struct GTY(()) bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 2;
    unsigned int flag3 : 3;
    int regular_field;
};

/* Variable length struct */
struct GTY(()) var_len_struct {
    int count;
    int items GTY((length("%h.count"))) [];
};

/* Opaque pointer type */
typedef struct GTY(()) opaque_struct* opaque_ptr;

#endif /* TEST_TYPES_H */
