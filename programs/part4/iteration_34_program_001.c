/* test_types.h - Type definitions to cover all gengtype type categories */

#ifndef TEST_TYPES_H
#define TEST_TYPES_H

#include <stddef.h>

/* TYPE_SCALAR: Basic scalar type */
typedef int my_scalar GTY(());

/* TYPE_STRING: String type with length annotation */
struct string_struct GTY(()) {
    char * GTY((length("str_len"))) data;
    int str_len;
};

/* TYPE_STRUCT: Plain C struct */
struct plain_struct GTY(()) {
    int field1;
    double field2;
    my_scalar scalar_field;
};

/* TYPE_USER_STRUCT: User-defined struct with custom marker */
struct user_defined GTY((user)) {
    void *user_data;
    int user_tag;
};

/* TYPE_UNION: Union type */
union my_union GTY(()) {
    int int_val;
    double double_val;
    char * GTY((tag("0"))) string_val;
    struct plain_struct * GTY((tag("1"))) struct_ptr;
};

/* TYPE_POINTER: Pointer types */
struct pointer_container GTY(()) {
    struct plain_struct * GTY((skip)) direct_ptr;
    struct string_struct ** GTY((skip)) double_ptr;
    void * GTY((skip)) generic_ptr;
};

/* TYPE_ARRAY: Array types */
struct array_container GTY(()) {
    /* Fixed-size array */
    int fixed_array[10];
    
    /* Variable-length array with length annotation */
    char * GTY((length("vla_len"))) variable_array;
    size_t vla_len;
    
    /* Array of pointers */
    struct plain_struct * GTY((length("ptr_array_len"))) ptr_array[5];
    int ptr_array_len;
};

/* TYPE_CALLBACK: Callback/function pointer type */
typedef void (*callback_func)(int, const char *) GTY((callback));

struct callback_container GTY(()) {
    callback_func handler;
    void * GTY((skip)) user_data;
};

/* TYPE_LANG_STRUCT: Language-specific struct */
#ifdef __cplusplus
extern "C" {
#endif

struct lang_specific GTY((lang_struct)) {
    int lang_field1;
    void * GTY((skip)) lang_field2;
};

#ifdef __cplusplus
}
#endif

/* Complex nested type to ensure deep traversal */
struct nested_container GTY(()) {
    struct plain_struct base;
    union my_union choice;
    struct array_container arrays;
    struct pointer_container pointers;
    struct callback_container callbacks;
    struct string_struct str_data;
};

/* Forward declaration for circular reference */
struct forward_declared GTY(());

struct circular_ref GTY(()) {
    struct forward_declared *next;
    int value;
};

struct forward_declared GTY(()) {
    struct circular_ref *prev;
    char name[32];
};

/* TYPE_UNDEFINED: This should be triggered by incomplete types */
/* Note: gengtype should encounter undefined types during processing */

/* Additional complex types for thorough coverage */

/* Bitfield struct */
struct bitfield_struct GTY(()) {
    unsigned int flag1 : 1;
    unsigned int flag2 : 2;
    unsigned int flag3 : 3;
    int regular_field;
};

/* Struct with attribute */
struct attributed_struct GTY(()) {
    int data __attribute__((aligned(16)));
    volatile double measurement;
    const char *constant_ptr;
};

/* Anonymous union within struct */
struct with_anonymous_union GTY(()) {
    int type;
    union {
        int int_value;
        double double_value;
        struct plain_struct *struct_ptr;
    } value;
};

/* Packed struct */
struct packed_struct GTY(()) {
    char a;
    int b;
    double c;
} __attribute__((packed));

#endif /* TEST_TYPES_H */
