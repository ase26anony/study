#ifndef TEST_TYPES_H
#define TEST_TYPES_H

#include <stddef.h>

/* TYPE_SCALAR: Basic scalar type */
typedef int my_scalar GTY(());
typedef unsigned long another_scalar GTY(());

/* TYPE_STRING: String type with length annotation */
struct GTY(()) string_struct {
    char * GTY((length("str_len"))) data;
    size_t str_len;
};

/* TYPE_STRUCT: Plain C struct */
struct GTY(()) plain_struct {
    int x;
    double y;
    my_scalar z;
};

/* TYPE_USER_STRUCT: User-defined struct with custom marker */
struct GTY((user)) user_defined_struct {
    void *custom_data;
    int user_tag;
};

/* TYPE_UNION: Union type */
union GTY(()) tagged_union {
    int int_val;
    double double_val;
    char * GTY((tag("0"))) string_val;
    struct plain_struct * GTY((tag("1"))) struct_ptr;
};

/* TYPE_POINTER: Pointer types */
struct GTY(()) pointer_container {
    struct plain_struct * GTY(()) ptr_to_struct;
    union tagged_union * GTY(()) ptr_to_union;
    struct string_struct ** GTY(()) ptr_to_ptr;
    void (* GTY(()) func_ptr)(void);
};

/* TYPE_ARRAY: Array types */
struct GTY(()) array_container {
    int fixed_array[10];
    char * GTY((length("dynamic_len"))) dynamic_array;
    size_t dynamic_len;
    struct plain_struct struct_array[5];
};

/* TYPE_CALLBACK: Callback/function pointer type */
typedef void (* GTY((callback))) callback_func(int, const char*);

struct GTY(()) callback_container {
    callback_func * GTY(()) handler;
    void (* GTY((callback))) direct_callback(double);
};

/* TYPE_LANG_STRUCT: Language-specific struct */
#ifdef __cplusplus
extern "C" {
#endif

struct GTY((lang_struct)) language_specific {
    int language_id;
    void* language_data;
};

#ifdef __cplusplus
}
#endif

/* Complex nested type to ensure deep traversal */
struct GTY(()) complex_nested {
    struct pointer_container * GTY(()) pointers;
    struct array_container arrays;
    union tagged_union variant;
    callback_func * GTY(()) callback;
    struct GTY(()) inner_struct {
        int depth;
        struct inner_struct * GTY(()) next;
    } * GTY(()) recursive;
};

/* Forward declaration for circular reference */
struct GTY(()) forward_declared;
struct GTY(()) forward_declared {
    int value;
    struct forward_declared * GTY(()) next;
};

/* TYPE_UNDEFINED: Should be caught by incomplete types */
struct GTY(()) incomplete_struct;
extern struct incomplete_struct * GTY(()) external_ref;

/* Template-like macro for generating multiple struct types */
#define DECLARE_STRUCT_TYPE(name, field_type) \
    struct GTY(()) name##_struct { \
        field_type value; \
        struct name##_struct * GTY(()) next; \
    }

DECLARE_STRUCT_TYPE(int_list, int);
DECLARE_STRUCT_TYPE(double_list, double);
DECLARE_STRUCT_TYPE(string_list, char*);

/* Attribute usage for GCC extensions */
struct GTY(()) attributed_struct 
    __attribute__((aligned(16))) 
    __attribute__((packed)) {
    char c;
    int i __attribute__((aligned(8)));
    double d;
};

/* Variable length array in struct */
struct GTY(()) vla_container {
    int count;
    int data[] __attribute__((aligned(32)));
};

#endif /* TEST_TYPES_H */
