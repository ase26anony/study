#ifndef TEST_TYPES_H
#define TEST_TYPES_H

#include <stddef.h>

/* TYPE_SCALAR: Basic scalar type definitions */
typedef int my_scalar;
typedef long my_long_scalar;
typedef unsigned char my_uchar_scalar;

/* TYPE_STRING: String type with GTY annotation */
struct GTY(()) string_struct {
    char * GTY((length("str_len"))) data;
    size_t str_len;
};

/* TYPE_STRUCT: Plain C struct with GTY tag */
struct GTY(()) plain_struct {
    int field1;
    double field2;
    my_scalar field3;
};

/* TYPE_USER_STRUCT: User-defined struct with custom marker */
struct GTY((user)) user_defined_struct {
    void *custom_data;
    int (*custom_callback)(void);
};

/* TYPE_UNION: Union containing GTY-tagged fields */
union GTY(()) tagged_union {
    struct GTY((tag("0"))) {
        int int_value;
    } as_int;
    struct GTY((tag("1"))) {
        double double_value;
    } as_double;
    struct GTY((tag("2"))) {
        char * GTY((length("str_len"))) string_value;
        size_t str_len;
    } as_string;
};

/* TYPE_POINTER: Struct containing pointers to other GTY-tagged types */
struct GTY(()) pointer_container {
    struct plain_struct * GTY((skip)) ptr_to_struct;
    union tagged_union * GTY((skip)) ptr_to_union;
    struct string_struct ** GTY((skip)) ptr_to_ptr_string;
};

/* TYPE_ARRAY: Structs containing arrays */
struct GTY(()) fixed_array_struct {
    int GTY((length("10"))) fixed_array[10];
};

struct GTY(()) variable_array_struct {
    int GTY((length("array_len"))) *variable_array;
    size_t array_len;
};

struct GTY(()) nested_array_struct {
    struct plain_struct GTY((length("5"))) struct_array[5];
    int count;
};

/* TYPE_CALLBACK: Function pointer/callback types */
typedef int (* GTY((callback))) compare_func_t(const void *, const void *);

struct GTY(()) callback_container {
    compare_func_t *comparator;
    void * GTY((skip)) user_data;
};

/* TYPE_LANG_STRUCT: Language-specific struct with hooks */
#ifdef __cplusplus
extern "C" {
#endif

struct GTY((lang_struct)) language_specific {
    void * GTY((skip)) language_data;
    int language_tag;
    
    /* Language-specific hooks */
    void (* GTY((callback))) init_hook(void);
    void (* GTY((callback))) cleanup_hook(void);
};

#ifdef __cplusplus
}
#endif

/* Complex type graph to ensure thorough traversal */
struct GTY(()) complex_node {
    struct complex_node * GTY((skip)) next;
    struct complex_node * GTY((skip)) prev;
    union tagged_union data;
    struct fixed_array_array_container * GTY((skip)) array_ref;
};

struct GTY(()) fixed_array_array_container {
    struct complex_node GTY((length("8"))) nodes[8];
    int active_count;
};

/* Forward declarations to create circular references */
struct GTY(()) forward_decl_struct;
typedef struct forward_decl_struct forward_decl_t;

struct GTY(()) forward_decl_struct {
    forward_decl_t * GTY((skip)) self_ptr;
    struct forward_decl_struct * GTY((skip)) another;
    int value;
};

/* TYPE_UNDEFINED: This might be triggered by incomplete types or special cases */
/* We'll create a typedef to an undefined struct */
typedef struct undefined_struct undefined_type_t;

/* Another approach: use a struct with a field of undefined type */
struct GTY(()) has_undefined_field {
    int valid_field;
    /* This might trigger TYPE_UNDEFINED during processing */
    struct not_defined_yet * GTY((skip)) undefined_ptr;
};

#endif /* TEST_TYPES_H */
