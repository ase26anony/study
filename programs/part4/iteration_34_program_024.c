/* test_types.h - Comprehensive type definitions for gengtype coverage */

#ifndef TEST_TYPES_H
#define TEST_TYPES_H

#include <stddef.h>

/* TYPE_SCALAR: Basic scalar type definitions */
typedef int my_scalar;
typedef long my_long_scalar;
typedef unsigned char my_uchar_scalar;

/* TYPE_STRING: String type with length annotation */
struct GTY(()) string_struct {
    char * GTY((length("str_len"))) data;
    size_t str_len;
};

/* TYPE_STRUCT: Plain C structs with GTY tags */
struct GTY(()) simple_struct {
    int field1;
    double field2;
    my_scalar field3;
};

/* Nested struct for complex type graph */
struct GTY(()) outer_struct {
    struct simple_struct * GTY((skip)) inner;
    int count;
};

/* TYPE_USER_STRUCT: User-defined struct with custom markers */
struct GTY((user)) user_defined_struct {
    int user_id;
    void * GTY((skip)) user_data;
    const char *user_name;
};

/* TYPE_UNION: Union containing GTY-tagged fields */
union GTY(()) tagged_union {
    int int_value;
    double double_value;
    struct simple_struct * GTY((skip)) struct_ptr;
    char * GTY((length("strlen((char*)&union_string)"))) union_string;
};

/* TYPE_POINTER: Struct containing various pointers */
struct GTY(()) pointer_container {
    struct simple_struct * GTY((skip)) direct_ptr;
    struct outer_struct ** GTY((skip)) double_ptr;
    void * GTY((skip)) void_ptr;
    const char * GTY((skip)) const_string_ptr;
};

/* TYPE_ARRAY: Structs with different array types */
struct GTY(()) array_struct {
    /* Fixed-size array */
    int fixed_array[10];
    
    /* Variable-length array with length annotation */
    int * GTY((length("var_len"))) variable_array;
    size_t var_len;
    
    /* Array of pointers */
    struct simple_struct * GTY((skip)) ptr_array[5];
};

/* TYPE_CALLBACK: Function pointer types and callback-annotated structs */
typedef void (*callback_func)(int, void*);

struct GTY((callback("my_callback"))) callback_struct {
    int callback_id;
    callback_func GTY((skip)) handler;
    void * GTY((skip)) context;
};

/* Another callback type using different annotation */
typedef int (*compare_func)(const void*, const void*);

struct GTY(()) uses_callback {
    compare_func GTY((skip)) comparator;
    void * GTY((skip)) data;
    size_t data_size;
};

/* TYPE_LANG_STRUCT: Language-specific struct hooks */
#ifdef __cplusplus
extern "C" {
#endif

/* Using lang_struct annotation for language-specific handling */
struct GTY((lang_struct("C"))) lang_specific_struct {
    int lang_specific_field;
    void * GTY((skip)) lang_data;
};

/* Another approach with chain_next/chain_prev for linked structures */
struct GTY((chain_next("%h.next"), chain_prev("%h.prev"))) linked_node {
    int value;
    struct linked_node * GTY((skip)) next;
    struct linked_node * GTY((skip)) prev;
};

#ifdef __cplusplus
}
#endif

/* Complex nested type combining multiple categories */
struct GTY(()) master_container {
    /* Scalar */
    my_scalar scalar_field;
    
    /* String */
    struct string_struct GTY((skip)) str_field;
    
    /* Struct */
    struct simple_struct GTY((skip)) nested_struct;
    
    /* User struct */
    struct user_defined_struct * GTY((skip)) user_struct_ptr;
    
    /* Union */
    union tagged_union GTY((skip)) union_field;
    
    /* Pointer */
    struct pointer_container * GTY((skip)) ptr_container;
    
    /* Array */
    struct array_struct GTY((skip)) array_field;
    
    /* Callback */
    struct callback_struct GTY((skip)) callback_field;
    
    /* Language struct */
    struct lang_specific_struct * GTY((skip)) lang_struct_ptr;
    
    /* Linked list using chain_next */
    struct linked_node * GTY((skip)) list_head;
};

/* TYPE_UNDEFINED: Forward declarations that create undefined types */
struct GTY(()) forward_declared;  /* This creates TYPE_UNDEFINED */

/* Later definition */
struct GTY(()) forward_declared {
    int defined_field;
    struct forward_declared * GTY((skip)) self_ptr;
};

/* Template-like macro to generate multiple instances */
#define DECLARE_GTY_STRUCT(name, field_type) \
    struct GTY(()) name { \
        field_type GTY((skip)) data; \
        struct name * GTY((skip)) next; \
    }

/* Generate several struct types */
DECLARE_GTY_STRUCT(gty_list_int, int);
DECLARE_GTY_STRUCT(gty_list_double, double);
DECLARE_GTY_STRUCT(gty_list_ptr, void*);

/* Opaque pointer type */
typedef struct GTY(()) opaque_struct *opaque_handle;

#endif /* TEST_TYPES_H */
