#ifndef TEST_TYPES_H
#define TEST_TYPES_H

#include <stddef.h>

/* TYPE_SCALAR: Basic scalar type */
typedef int GTY(()) my_scalar;

/* TYPE_STRING: String type with length callback */
struct GTY(()) string_struct {
    char * GTY((length("str_len"))) data;
    size_t str_len;
};

/* TYPE_STRUCT: Plain struct type */
struct GTY(()) plain_struct {
    int x;
    double y;
    my_scalar z;
};

/* TYPE_USER_STRUCT: User-defined struct with custom marker */
struct GTY((user)) user_struct {
    void *data;
    int tag;
};

/* TYPE_UNION: Union type */
union GTY(()) my_union {
    int i;
    double d;
    char * GTY((tag("0"))) s;
    struct plain_struct * GTY((tag("1"))) ps;
};

/* TYPE_POINTER: Pointer types */
struct GTY(()) pointer_struct {
    struct plain_struct * GTY(()) ptr1;
    struct string_struct * GTY(()) ptr2;
    void * GTY((skip)) opaque_ptr;
};

/* TYPE_ARRAY: Array types */
struct GTY(()) array_struct {
    int GTY(()) fixed_array[10];
    int * GTY((length("array_len"))) variable_array;
    size_t array_len;
    
    /* Nested array in struct */
    struct plain_struct GTY(()) nested_array[5];
};

/* TYPE_CALLBACK: Callback function type */
typedef void (* GTY((callback)) callback_func)(void *data, int value);

struct GTY(()) callback_container {
    callback_func GTY(()) handler;
    void * GTY(()) user_data;
};

/* TYPE_LANG_STRUCT: Language-specific struct */
struct GTY((lang_struct)) lang_specific {
    int language_specific_field;
    void * GTY((desc("1"))) language_data;
};

/* Complex nested type to ensure deep traversal */
struct GTY(()) complex_nested {
    struct pointer_struct * GTY(()) ptr_field;
    union my_union GTY(()) union_field;
    struct array_struct GTY(()) array_field;
    struct string_struct GTY(()) string_field;
};

/* Forward declarations to create type cycles */
struct GTY(()) forward_decl;
struct GTY(()) forward_container {
    struct forward_decl * GTY(()) next;
};

struct GTY(()) forward_decl {
    int value;
    struct forward_container * GTY(()) container;
};

/* TYPE_UNDEFINED: This might be triggered by incomplete types or errors */
/* We'll create a situation where gengtype might encounter undefined types */
struct GTY(()) undefined_container {
    struct incomplete_type * GTY(()) undefined_ptr;  /* Forward declared but never defined */
};

/* Another way: use a typedef to an undefined struct */
typedef struct never_defined GTY(()) undefined_alias;

#endif /* TEST_TYPES_H */
